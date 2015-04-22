#include "main.h"
#include <cmath>
#include <string>
#include <algorithm>


bool MyApp::Menu_Extraction_CorrelationCoefficient_PersonalizedPlate( Image &image )
{
    if ( image.IsNull() ) return false;     //checks if the image is valid

    Image copy(image);
    iterativeBinaryThreshold( copy );

    //initalize an array to keep track of found numbers or letters
    double timeElapse = 0;
    int plateCols[7] = { -1 };
    char plateValues[7] = { ' ' };
    
	//start time
	clock_t start = clock();

    //call the extraction algorithm
    correlationExtraction( copy, plateValues, plateCols, 7 );

	//end time
	clock_t end = clock();

	timeElapse = double(end - start) / CLOCKS_PER_SEC;
        
	//order the output according to col position
	orderPlateValues( plateValues, plateCols, timeElapse );
	
    //display alpha-numeric sequence
	//display time taken
	return true;
}

bool MyApp::Menu_Extraction_CorrelationCoefficient_StandardPlate( Image &image )  
{
    if ( image.IsNull() ) return false;     //checks if the image is valid

	double timeElapse = 0;

    Image copy(image);
    iterativeBinaryThreshold( copy );

    //initalize an array to keep track of found numbers or letters
    int plateCols[7] = { 0 };
    char plateValues[7] = { ' ' };
    
	//start time
	clock_t start = clock();

    //call the extraction algorithm
    correlationExtraction( copy, plateValues, plateCols, 6 );

	//end time
	clock_t end = clock();

	timeElapse = double(end - start) / CLOCKS_PER_SEC;
        
	//order the output according to col position
	orderPlateValues( plateValues, plateCols, timeElapse );
	
    //display alpha-numeric sequence
	//display time taken
	return true;
}


void MyApp::correlationExtraction( Image &image, char plateValues[], int plateCols[], int num )
{
    bool check = true;
    bool valFound = false;
    int numDetected = 0;
    int nrows = image.Height();             //saves the height dimensions of the image
    int ncols = image.Width();              //saves the width dimesnions of the image
    float maskAverage = 0.0;                  //used to compuete average intensity of mask area
    float ImgNeighborhoodAvg = 0.0;           //used to compuete average intensity of image neighboorhood area
    float numeratorSum = 0.0;
    float denominatorSum = 0.0;
    float denominator1 = 0.0;
    float denominator2 = 0.0;
    float correlation = 0.0;
    int maskRow = 0;
    int maskCol = 0;
    int maskSize = 0;
    string CorImgLabel;
    string maskVersion[] = { "templates100", "templates80", "", "-2" };
    string maskValue[] = { "0", "2", "3", "4", "5", "6", "7", "8", "9", 
			   "A", "B", "C", "D", "E", "F", "G", "H", "J", 
			   "K", "M", "N", "O", "P", "Q", "R", "S", 
			   "U", "V", "W", "X", "Y", "Z", "T", "L", "I", "1" };
    
/*---loop through all mask versions---*/
for( int MV = 0; MV < 2; MV++)
{
    /*---loop through all mask templates---*/
    for ( int ML = 0; ML < 36; ML++)
    {
        //sets string for new correlated values image
        CorImgLabel = "Mask = "; 
       
        /*---Read in a template from file---*/
        string name = "../images/" + maskVersion[MV] + "/" + maskValue[ML] + maskVersion[MV+2] + ".JPG";
        //string name = maskValue[ML] + ".JPG";
        Image mask( name );
        //checks if the image is valid
        if ( mask.IsNull() )
        {
            continue;   //go to next template if image is invalid
        }   

        //Prints to the cosole what mask is being processed
        cout << "Running Mask: " << maskValue[ML] + maskVersion[MV+2] << endl;

        //gets current mask values
        maskRow = mask.Height();
        maskCol = mask.Width();
        maskSize = (maskRow * maskCol);

        //copies the current image to write the correlated values to later
        Image XCorImg( image );
        XCorImg.Fill(Pixel(0,0,0));

	    /*---compute maskAverage for specific mask used---*/
        for ( int i = 0; i < maskRow; i++ )
        {
            for( int j = 0; j < maskCol; j++ )
            {
              maskAverage += mask[i][j]; //sum all the values in the mask
            }
        }
        maskAverage /= maskSize; //average the sum by dividing the num elements in mask
	
	    //---loops through image and applies correlation algorithm---
        for ( int r = 0; r < (nrows - maskRow); r++ )
	    {
            for ( int c = 0; c < (ncols - maskCol); c++ )
            {	
		        //---precompute ImgNeighborhoodAvg which is size of template---
                //    printf ( "Working on row %d and on column %d.\n", r, c);
		        for ( int i = 0; i < maskRow; i++ )
		        {
		            for( int j = 0; j < maskCol; j++ )
		            {
		                ImgNeighborhoodAvg += image[r+i][c+j]; //sum the intensity values in the neighborhood
		            }
		        }
		        ImgNeighborhoodAvg /= maskSize; //divides the average by the number of mask elements (since mask size = neighborhood size)
   
		        //---loops through each mask and apply the correlation algorithm---
			    for ( int x = 0; x < maskRow; x++ )
			    {
				    for ( int y = 0; y < maskCol; y++ )
				    {
					//compute numerator
					numeratorSum += ((mask[x][y] - maskAverage) * (image[r+x][c+y] - ImgNeighborhoodAvg));
			
                        		//computes each section of the denominator seperately
                        		denominator1 += ((mask[x][y] - maskAverage) * ( mask[x][y] - maskAverage ));
                        		denominator2 += ((image[r+x][c+y] - ImgNeighborhoodAvg) * ( image[r+x][c+y] - ImgNeighborhoodAvg ));
				    }
		            }
                //compute denominator
                denominatorSum = sqrt(denominator1 * denominator2); 
                                                
			    //check for divide by 0 error
			    if (denominatorSum == 0.0)
			    {
			        correlation = 0.0; //set correlation to zero if dvision by is to occur (semi arbitrarily )
			    }
			    else //calculates correlation
			    {
			        correlation = (numeratorSum/denominatorSum);
			        //cout << correlation << endl;
			    }
                //sets the correlation image with the found correlated values
			    XCorImg[r][c].SetGray(abs((int)(correlation * 255)));
                
                //determines positive match with image and template
			    if (correlation >= 0.75)
			    {
				    valFound = true;
                    //draws letter at correlation match
                    //XCorImg.DrawText(r+maskRow, c, maskValue[ML], Pixel(0,255,255), Image::Horizontal);

                    //save the column position and mask value to the 2D array
			        //if list is empty 
			        if ( numDetected == 0 )
			        {
			            plateCols[numDetected] = c;
			            plateValues[numDetected] = maskValue[ML][0];
			            numDetected = 1;
				    cout << "Added to Plate." << endl;
			        }   
			        else //save the matches in the array
			        {
                        //checks if the template match is within 3 pixels, to eliminate redundant matches
			            if ( /*plateValues[numDetected-1] != maskValue[ML][0] ||*/ abs( c - plateCols[numDetected-1] ) > (maskCol/1.5) )
			            {   
					if (plateValues[numDetected-1] == maskValue[ML][0])
					{
					    for ( int val = 0; val < 7; val++ )
					    {
						if( plateValues[val] == maskValue[ML][0] && abs( c - plateCols[val] ) < (maskCol/1.5))
						{
							check = false;
						}
					    }
					    if ( check == true )
					    {
			                            plateCols[numDetected] = c;
			                            plateValues[numDetected] = maskValue[ML][0];
			                            numDetected = numDetected + 1;
				    		    cout << "Added to Plate." << endl;
					    }
					check = true;
					} 
					else
					{
			                    plateCols[numDetected] = c;
			                    plateValues[numDetected] = maskValue[ML][0];
			                    numDetected = numDetected + 1;	
				    	    cout << "Added to Plate." << endl;
					}
			            }
			        }
			    }
                //resets the variables for next run through
                numeratorSum = 0.0;
                denominatorSum = 0.0;
                denominator1 = 0.0;
                denominator2 = 0.0;
                correlation = 0.0;
                ImgNeighborhoodAvg = 0;


	        //exits the processing of the plate image if 7 characters are already found
		if ( numDetected >= num )
		{
        	    CorImgLabel += maskValue[ML];
        	    displayImage(XCorImg, CorImgLabel);
		    return;
		}

		}
		}

		if ( valFound )
		{
        	//displays the correlation coefficients in a new image for each mask
        	CorImgLabel += maskValue[ML];
        	displayImage(XCorImg, CorImgLabel);
			valFound = false;
		}

        maskAverage = 0.0;   //reset the mask average
        maskSize = 0;
    } 
    
}                                           
}

void MyApp::orderPlateValues( char plateValues[], int plateCols[], double timeElapse )
{
	ostringstream convertTime;
	convertTime << timeElapse;
	Image textbox(100, 400);
        textbox.Fill(Pixel(0,0,0));

	string message = "";
    char license[7] = {'&'};
    int positions[7] = { plateCols[0], plateCols[1], plateCols[2], plateCols[3], 
                         plateCols[4], plateCols[5], plateCols[6] };
                         
    //sort the array of values and column positions                         
    sort( positions, positions + 7 );

    //DEBUG
    cout <<"Column Positions: "<< positions[0] <<' '<< positions[1] <<' '
	 << positions[2] <<' ' << positions[3] <<' '<< positions[4] <<' '
	 << positions[5] <<' ' << positions[6] << endl;

    //Determine order of characters in license plate
    for ( int i = 0; i < 7; i++ )
    {
        for ( int j = 0; j < 7; j++ )
        {
            if ( plateCols[j] == positions[i] )
                license[i] = plateValues[j];
        }
	}

	message += "Plate Values: ";
	for ( int p = 0; p < 7; p++ )
	{
		message.push_back( license[p] );
		//message.insert( ' ' );
	}
	
	textbox.DrawText( 10, 70, message, Pixel(255, 255, 255));	

	message = "Time Elapsed: " + convertTime.str();

	textbox.DrawText(10, 50, message, Pixel(255, 255, 255));

        displayImage(textbox, "Sequence Extracted");
    ////DEBUG	
    cout << "Plate Values:           " << plateValues[0] << plateValues[1] 
	 << plateValues[2] << plateValues[3] << plateValues[4] << plateValues[5] << plateValues[6] << endl;
    //cout << "Plate Values (ordered): ";
    
    //print the characters in the array
	//for ( int p = 0; p < 7; p++ )
	//{
	//    cout << license[p];
    //}
    //cout << endl;
}
