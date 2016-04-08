#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
	FILE *ifp, *ofp; //File pointers for different files

	char geofence[10000];
	int index = 0;
	int index2 = 0;

	/*ifp = fopen("geofenceData.txt", "w");
	fprintf(ifp, "d");
	fclose(ifp);*/



	ifp = fopen("geofenceData.txt", "r"); //Reads in geofence data based on KML output
	ofp = fopen("geofence.txt", "w"); //Outputs in format autopilot/qGroundControl can understand

	fprintf(ofp, "0, 457 \n"); //Initially print out the minimum and maximum altitude of the plane, 457 is 1500ft in metres



	//And now we read data from geofenceData
	while(!feof(ifp)){
		geofence[index] = fgetc(ifp);
		index++;
	}

	printf("%d", index);



	while(index2 < (index-1)){
		printf("%d, %c\n", index2, geofence[index2]);
		if(geofence[index2] == ',') {
			printf("%c", geofence[index2+1]);
			if(geofence[index2+1] == '0'){
				printf("new coordinates");
				fprintf(ofp, "\n");
				index2 = index2 + 3;
			} else {
				fprintf(ofp, " ");
				index2++;
			}
		} else {
			fprintf(ofp, "%c", geofence[index2]);
			index2++;
		}

	}



	printf("hello world");
	fclose(ifp);
	fclose(ofp);


	return 1;
}
