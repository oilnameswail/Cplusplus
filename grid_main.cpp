/*==================GRID===================*/

#include <fstream>
#include <map>
#include <math.h>
//#include <boost/multi_array.hpp>  That's right. I didn't.
#include <boost/thread/thread.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

/*------------------------------------------------------------------------------------------
------------ Hello World -------------------------------------------------------------------
--------------------------------------------------------------------------------------------*/

void introduction()
{
    cout << "-----------------------------------------------------------" << endl;
    cout << "                    This is Downscale" << endl;
    cout << "-----------------------------------------------------------" << endl;
    cout << "Assuming lengths that are always factorable by 2 (even)," << endl;
    cout << "a geometric series can be used to determine a method " << endl;
    cout << "with a scale factor of 1/2." << endl;
    cout << "" << endl;
    cout << "s = a * r^n" << endl;
    cout << "" << endl;
    cout << "where" << endl;
    cout << "" << endl;
    cout << "s = # of segments in resulting dimension" << endl;
    cout << "a = original length of side" << endl;
    cout << "r = 1/2" << endl;
    cout << "" << endl;
    cout << "Consequentally, in this scenario" << endl;
    cout << "r^(-n) = length of the segment" << endl;
    cout << "-----------------------------------------------------------" << endl;
    cout << "" << endl;
}



/*------------------------------------------------------------------------------------------
------- Iterations - determine grid limit --------------------------------------------------
------- Useful for when symetry is not true in all dimensions ------------------------------
--------------------------------------------------------------------------------------------*/
int iterations(int a, int b, int c) /*Pass in dimensions of the array*/
{
    double r = 0.5;
    int iter=0; /*will hold the smallest value of m, n, or o*/
    /*
    m = index in x-direction
    n = index in y-direction
    o = index in z-direction
    */

    int m=1;
    int n=1;
    int o=1;

    //a=8, b=4, c=4 for testing
    int s = a * pow(r,m);
    int p = b * pow(r,n);
    int q = c * pow(r,o);

    //determine limiting n, m and o values.
    while (s>1)
    {
        m++;
        s = a * pow(r,m);
    }
    while (p>1)
    {
        n++;
        p = b * pow(r,n);
    }
    while (q>1)
    {
        o++;
        q = c * pow(r,o);
    }

    int smallest = m;
    if (n < smallest) {smallest = n;}
    if (o < smallest) {smallest = o;}

    iter=smallest;
    //cout<<"OG dimensions: "<<a<<"x"<<b<<"x"<<c<<endl;
    //cout<<"Iterations:         "<<iter<<endl;
    return iter; //take limit = [1]-->[iter] (n=0 is special, "eval whole case")
}



/*------------------------------------------------------------------------------------------
------- Function to evaluate the whole animal to determine most common value of whole ------
------------------------------------------------------------------------------------------*/

int eval_whole(int ****pStartArray3D, int a, int b, int c, int g) //g=0.
{

    int count_1=0;
    int count_2=0;
    int result;
    for(int t=0; t<g; t++)
    {
        for(int k=0; k<c; k++)
        {
            for(int j=0; j<b; j++)
            {
                for(int i=0; i<a; i++)
                {
                    switch(pStartArray3D[t][i][j][k])
                    {
                        case 1:
                            {count_1++;
                            break;}
                        case 2:
                            {count_2++;
                            break;}
                    }
                }
            }
        }
    }
    if(count_1>count_2) {result=1;}
    if(count_1<count_2) {result=2;}
    if(count_1==count_2)
     {
       //cout<<"How bout that.  There is an even dist. of 1's and 2's. Let fate decide. "<<endl;
       result=rand() %2+1;
      }
    return result;
}


/*----------------------------------------------------------------------------------------
------- GRID FUNCTION --------------------------------------------------------------------
------- where ----------------------------------------------------------------------------
-------- -> t = "snapshot" of result at a particular grid, -------------------------------
--------     --> eliminates race conditions by storing result arrays in own place --------
-------- -> a, b, and c are all dimensions of the original 3D array ----------------------
-------- -> gridI = iteration-size of original array (when not a cube), ------------------
--------     -->  based on limit determined in iterations() func'n. ----------------------
--------     -->  will iterate through gridI=1 to gridI<=iter in main() ------------------
-------- -> pStartArray3D is a pointer to the array being evaluated ----------------------
------------------------------------------------------------------------------------------*/


void grid(int ****pStartArray3D, int a, int b, int c, int gridI, int t, int ****pResult3D)
{
    //NOTE: time index of OGA will always be 0


    /* Unit testing
    cout<<"------------------------"<<endl;
	cout<<"---- Inside Grid -------"<<endl;
    cout<<"---- Zoom Level: g="<<gridI<<" ---"<<endl;
    cout<<"---- t="<<t<<" ---"<<endl;
    */

    double r=0.5; //..................based on assumption that this is an "even" geometic series, with a scale factor of (1/2)

    int s = a * pow(r,gridI); //......................# of segments [dimension of resulting array in x-direction]
    int x_segment_length = pow(2,gridI); //...........length of each segment
    int x_move=x_segment_length-1;; //................moves marker over by length (-1 for index accuracy)

    int p = b * pow(r,gridI); //......................# of segments [dimension of resulting array in y-direction]
    int y_segment_length = pow(2,gridI);  //..........length of each segment
    int y_move=y_segment_length-1; //.................moves marker over by length (-1 for index accuracy)

    int q = c * pow(r,gridI); //......................# of segments [dimension of resulting array in z-direction]
    int z_segment_length = pow(2,gridI);  //..........length of each segment
    int z_move=z_segment_length-1; //.................moves marker over by length (-1 for index accuracy)

    //assuming the 3D rectangular objects is populated with binary 1-2 (i.e., ea element has either 1 or 2).
    int count_1=0;
    int count_2=0;

    //indexes of grid
    int i=0;
    int j=0;
    int k=0;

    //pointers
    int x_beg = 0;
    int y_beg = 0;
    int z_beg = 0;
    int x_end = x_beg + x_move;
    int y_end = y_beg + y_move;
    int z_end = z_beg + z_move;

/*	---Unit testing------------
    cout << "s: " << s << endl;
    cout << "p: " << p << endl;
    cout << "q: " << q << endl;
    cout << "x_beg: " << x_beg << endl;
    cout << "y_beg: " << y_beg << endl;
    cout << "z_beg: " << z_beg << endl;
    cout << "x_seg_len: " << x_segment_length << endl;
    cout << "y_seg_len: " << y_segment_length << endl;
    cout << "z_seg_len: " << z_segment_length << endl;
    cout << "x_move: " << x_move << endl;
    cout << "y_move: " << y_move << endl;
    cout << "z_move: " << y_move << endl;
    cout << "x_end: " << x_end << endl;
    cout << "y_end: " << y_end << endl;
    cout << "z_end: " << z_end << endl;
*/

    while(k<q)
        {
            j=0;
            y_beg=0;
            y_end=y_beg+y_move;

            while (j<p)
            {
              i=0;
              x_beg=0;
              x_end = x_beg + x_move;

              while(i<s)
              {
                //reset counters
                count_1=0;
                count_2=0;

                for(int z=z_beg; z<=z_end; z++)
                {
                    for(int y=y_beg;  y<=y_end; y++)
                    {
                         for(int x=x_beg; x<=x_end; x++)
                         {
                            /*Move through the original array -- the original array is "timeless, so t=0 for OGA
                            */
                            switch (pStartArray3D[0][x][y][z])
                            {
                              case 1: count_1++;
                              break;
                              case 2: count_2++;
                              break;
                            }
                         }
                    }
                }
                if (count_1>count_2)
                    {pResult3D[t][i][j][k]=1;}
                if (count_1<count_2)
                    {pResult3D[t][i][j][k]=2;}
                if (count_1==count_2)
                    {pResult3D[t][i][j][k]=rand() %2+1;}

                x_beg = x_end + 1;
                x_end = x_beg + x_move;
                i++;
               }
              y_beg = y_end + 1;
              y_end = y_beg + y_move;
              j++;
            }
            z_beg = z_end + 1;
            z_end = z_beg + z_move;
            k++;
        }


        /*cout << "-------------------------------" << endl;
        cout << "Print RESULT ARRAY for sanity: " << endl;
        cout << "-------------------------------" << endl;
        for(int k=0; k<q; k++)
        {
            cout<<"--- x-y plane at z="<<k<<": ---"<<endl;
            for (int j=0; j<p; j++)
            {

                for (int i=0; i<s; i++)
                {
                    cout <<"(x,y)=("<<i<<","<<j<<")=";
                    cout <<"Result Matrix here:" << pResult3D[t][i][j][k]<<endl;
                }
            }
            cout << "" << endl;
        }*/
}


/*----------------------------------------------------------------------------------------
------- CREATION OF ORIGINAL GANGSTA -----------------------------------------------------
-----------------------------------------------------------------------------------------*/
//pointer to Original 3DArray (a x b x c) - will break up into grids to evaluate.  Populate with binary pair.
//does not assume that it has to be a cube - only that all sides are even (case when cube: a=b=c)
void create_Array3D_in_T(int ****a3D, int a, int b, int c, int time_dim) //creates a 3D array inside a pointer
{
    //srand(time(NULL));
    for(int t=0; t<time_dim; t++)
    {
        for(int k=0; k<c; k++)
        {
            for(int j=0; j<b; j++)
            {
                for(int i=0; i<a; i++)
                {
                    a3D[t][i][j][k]=rand() %2 + 1; //randomly generate 1's and 2's (un-uniform distribution)
                }
            }
        }
    }
        //return 0;  //populates array insde (and) the pointer to it.
}

/*----------------------------------------------------------------------------------------
----- You think this is some kind of game?------------------------------------------------
------------------------------------------------------------------------------------------
------- This is a game, right? -----------------------------------------------------------
------------------------------------------------------------------------------------------*/
void tell_me_a_story(int *a) /*3D array, [a]=[3] always*/
{
    cout<<"So.  Give me an even number for an x dimension."<<endl;
    cin>>a[0];
    cout<<"And another for the y dimension."<<endl;
    cin>>a[1];
    cout<<"What are we, flatlanders?"<<endl;
    cin>>a[2];
}

void lets_dance(int user_choice[3])
{
    tell_me_a_story(user_choice);
    cout<<""<<endl;
    cout<<"Did you enter any odd numbers?"<<endl;
    cout<<"That's cool, I just cant guarantee the accuracy of their downscaling."<<endl;
    cout<<"(x,y,z)=("<<user_choice[0]<<","<<user_choice[1]<<","<<user_choice[2]<<")"<<endl;
    cout<<""<<endl;

    //true=1, false=0.
    bool shall_we = ((user_choice[0]<=1) || (user_choice[1]<=1) || (user_choice[2]<=1));
    //cout<<"Shall we dance = "<<shall_we<<endl;
    if(shall_we==1)
    {
        cout<<"So, we're not flatlanders."<<endl;
        cout<<"And in case you entered 0x0x0,"<<endl;
        cout<<"how would one go about determining"<<endl;
        cout<<"the most common value of a zero-dimensional space?"<<endl;
        cout<<"I mean, nothing is in there."<<endl;
        cout<<"How do you take the most common value of nothing."<<endl;
        cout<<"Would that be infinity.  Or wait."<<endl;
        cout<<"How do we even know we exist?"<<endl;
        cout<<""<<endl;
        cout<<"Well, that's far beyond the existential scope of this program."<<endl;
        cout<<"I am only a program after all."<<endl;
        cout<<""<<endl;
        cout<<"But I thought I'd let you know that I have thought about this,"<<endl;
        cout<<"and I'd like you to pick something not so existentially confusing."<<endl;
        cout<<""<<endl;
        tell_me_a_story(user_choice);
        shall_we=((user_choice[0]==0) || (user_choice[1]==0) || (user_choice[2]==0));
    }
    while(shall_we==1)
    {
        cout<<"Srsly. C'mon now."<<endl;
        tell_me_a_story(user_choice);
    }
    cout<<""<<endl;
    cout<<"Alrighty. Lets see what I can come up with."<<endl;
    cout<<""<<endl;

}



/*----------------------------------------------------------------------------------------
------- ONE FUNCTION TO RULE THEM ALL ----------------------------------------------------
-----------------------------------------------------------------------------------------*/
int main()
{
    srand(time(NULL)); //comment out for testing
    double r=0.5;
    introduction();

/*============ To avoid user interaction, (1) Comment these out: ===========*/
    char answer;

    cout<<"Do you want me to merely create an"<<endl;
    cout<<"array of 128x128x128 and downscale it? (y/n)"<<endl;
    cout<<endl;
    cin>>answer;
    cout<<endl;
    int user_choose[3];
    int a;
    int b;
    int c;

    if(answer=='n' || answer=='N')
    {
        lets_dance(user_choose);
        a = user_choose[0];
        b = user_choose[1];
        c = user_choose[2];
    }
    else if(answer=='y' || answer=='Y')
    {
        a=128;
        b=128;
        c=128;
    }
/*============= (2) And comment these back in: =====================================*/
/*
        a=128;
        b=128;
        c=128;

*/

    ofstream myfile;
    myfile.open ("Downscale_results.txt");


    //there can only be ONE ORIGINAL (duh.  I mean c'mon.)
    int OG_index=1;

    int ****p_O3DArray;
    //allocate memory for this bad boy
    p_O3DArray=new int***[OG_index];
    for(int t=0; t<OG_index; t++)
    {
        p_O3DArray[t]=new int**[a];
        for(int i=0; i<a; i++)
        {
            p_O3DArray[t][i]=new int*[b];
            for(int j=0; j<b; j++)
            {
                p_O3DArray[t][i][j]=new int[c];
            }
        }
    }


    //populate pointer to the original 3D array, to live at index t (we shall call this "time")
    //for OG, t=0, and pointer to it is just 1 dimensional array
    create_Array3D_in_T(p_O3DArray, a, b, c, OG_index); //index = actual size.

    //VERIFY:
    for(int t=0; t<OG_index; t++)
    {
        myfile<<"-------------------------"<<endl;
        myfile<<"------ OG ARRAY ---------"<<endl;
        myfile<<"-------------------------"<<endl;
        for(int k=0; k<c; k++)
        {
            myfile<<"---- xy plane @ z="<<k<<" -----"<<endl;
            for(int j=0; j<b; j++)
            {
                for(int i=0; i<a; i++)
                {
                  myfile<<p_O3DArray[t][i][j][k];
                }
                myfile<<""<<endl;
            }
        }
    }
    myfile<<"-------------------------"<<endl;


/**--->>>RETROSPECTIVE: this is a silly function.  Each grid is taken on the whole.
    This is especially silly when considering a non-symmetrical "rectangular cube", where
    a comparison between this value and the "last" value is really a comparison of this and
    the "first" last value (which would be at t=0, x=0, y=0, z=0 in the last resulting array)**/

    //integer value of most common value of whole:
    /**int whole_res=eval_whole(p_O3DArray,a,b,c,OG_index); //result will be an integer;**/
    //cout<<"eval_whole result: "<<whole_res<<endl;


        //determine limit
        int limit=iterations(a,b,c);
        //cout<<"limit = "<<limit<<endl;


        //cout<<"About to create result arrays."<<endl;
        int ****p_res3DArray; //store the result of current thread

        //allocate memory for these bad boys
        p_res3DArray=new int***[limit]; //several "snapshots" at each grid.
        for(int t=0; t<limit; t++)
        {
            //cout<<"t="<<t<<endl;
            int s = a*pow(r,t+1);
            int p = b*pow(r,t+1);
            int q = c*pow(r,t+1);
            //cout<<"s="<<s<<endl;
            //cout<<"p="<<p<<endl;
            //cout<<"q="<<q<<endl;
            p_res3DArray[t]=new int**[s];
            for(int i=0; i<s; i++)
            {
                p_res3DArray[t][i]=new int*[p];
                for(int j=0; j<p; j++)
                {
                    p_res3DArray[t][i][j]=new int[q];
                }
            }
        }

        //set up array of threads
        boost::thread zeThreads[limit];

        //g (we shall call this "grid") will be the power in s = a * r^g, and want to go all the way to s=2 in this loop (1x1x1 eval'd at end)
        for(int g=1; g<=limit; g++)
        {
            /*dimesions of resulting array are determined in grid-function
            size (length) of each dimension is determined in grid-function*/

            /* But, to avoid race conditions of creating resulting arrays...must
            allocate memory for the 3D result array that the point_to_res3D will point to,
            and we need the dimensions of desired result*/

            int time_place=g-1; //will control the index of the snapshot pointers
            //cout<<"time_place="<<time_place<<endl;

/*            int dimX=a*pow(r,g);
            int dimY=b*pow(r,g);
            int dimZ=c*pow(r,g);
            //cout<<"dimX="<<dimX<<endl;
            //cout<<"dimY="<<dimY<<endl;
            //cout<<"dimZ="<<dimZ<<endl;

*/
            zeThreads[time_place]=boost::thread(grid,p_O3DArray, a, b, c, g, time_place, p_res3DArray);

        }

        for(int thread_i=0; thread_i<limit; thread_i++)
        {
            zeThreads[thread_i].join();
        }

/**RETROSPECTIVE! As stated above, comparing the last transformation to the evalution of the whole is silly,
   as each transformation is taken from the whole.  It is especially silly and nonsensical when given a non-
   symetrical "rectangular cube," as the comparison would be on the "first" last value.
   So, I've commented out the call to the eval_whole() function above,
   and removed the after-thought "comparison" between the "last" value and the "whole" value**/
        //go back through created arrays after threads run for output (don't steal anyone's thunder; avoid race conditions)
        for(int g=1; g<=limit; g++)
        {
            int snap_shot=g-1;

            int dimX=a*pow(r,g);
            int dimY=b*pow(r,g);
            int dimZ=c*pow(r,g);

            /**if(g!=limit)
            {**/
            myfile<<"---- RES ARRAY at t="<<snap_shot<<" ---"<<endl;
            myfile<<"-------------------------"<<endl;
            for(int k=0; k<dimZ; k++)
            {
                myfile<<"---> xy plane @ z="<<k<<" <----"<<endl;
                for(int j=0; j<dimY; j++)
                {
                    for(int i=0; i<dimX; i++)
                    {
                        myfile<<p_res3DArray[snap_shot][i][j][k];
                    }
                    myfile<<""<<endl;
                }
            }
            myfile<<"-------------------------"<<endl;
            /**}
            else
            {
                int can_it_be;
                myfile<<"---- RES ARRAY at t="<<snap_shot<<" ---"<<endl;
                myfile<<"-------------------------"<<endl;
                for(int k=0; k<dimZ; k++)
                {
                    myfile<<"---> xy plane @ z="<<k<<" <----"<<endl;
                    for(int j=0; j<dimY; j++)
                    {
                        for(int i=0; i<dimX; i++)
                        {
                            can_it_be=p_res3DArray[snap_shot][i][j][k];
                            myfile<<p_res3DArray[snap_shot][i][j][k];
                        }
                        myfile<<""<<endl;
                    }
                }
                myfile<<"-------------------------"<<endl;
                myfile<<"       ."<<endl;
                myfile<<"       ."<<endl;
                myfile<<"       ."<<endl;
                myfile<<"~~~> Plot Twist! <~~~~~~~"<<endl;

                myfile<<"Whole Animal is "<<whole_res<<endl;
                myfile<<endl;
                if(whole_res==can_it_be)
                {
                    myfile<<"Well that was anticlimactic. They are the same."<<endl;
                }
                else
                {
                    myfile<<"Oh, the huge manatee!"<<endl;
                }
            }**/

        }

        //free up memory of p_O3DArray
        for(int i=0; i<1; i++)
        {
            for(int j=0; j<a; j++)
            {
                 for(int k=0; k<b; k++)
                 {
                    delete [] p_O3DArray[i][j][k];
                 }
                delete [] p_O3DArray[i][j];
            }
            delete [] p_O3DArray[i];
        }
        delete [] p_O3DArray;

        //free up memorty of all resulting arrays in p_res3DArray
        for(int i=0; i<limit; i++)
        {
            int s = a*pow(r,i+1);
            int p = b*pow(r,i+1);
            int q = c*pow(r,i+1);
            for(int j=0; j<s; j++)
            {
                 for(int k=0; k<p; k++)
                 {
                    delete [] p_res3DArray[i][j][k];
                 }
                delete [] p_res3DArray[i][j];
            }
            delete [] p_res3DArray[i];
        }
        delete [] p_res3DArray;


cout<<"BAM.  Made you a file. It's sitting right next to me."<<endl;

myfile.close();

return 0;



/*~~~~~Fin~~~~~*/

}

