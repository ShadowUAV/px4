#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
	FILE *ifp, *ofp; //File pointers for different files

	char geofence[10000];
	int index = 0;

	/*ifp = fopen("geofenceData.txt", "w");
	fprintf(ifp, "d");
	fclose(ifp);*/



	ifp = fopen("geofenceData.txt", "r"); //Reads in geofence data based on KML output
	ofp = fopen("geofence.txt", "w"); //Outputs in format autopilot/qGroundControl can understand

	fprintf(ofp, "0, 457 \n"); //Initially print out the minimum and maximum altitude of the plane, 457 is 1500ft in metres



	//And now we read data from geofenceData
	fscanf(ifp, "%[^\n]%s", geofence);
	while(geofence[index] != EOF){
		printf("%d, %c\n", index, geofence[index]);
		if(	geofence[index] == 'd'){
			printf("break \n");
			break;
		}
		if(geofence[index] == ',') {
			printf("%c", geofence[index+1]);
			if(geofence[index+1] == '0'){
				printf("new coordinates");
				fprintf(ofp, "\n");
				index = index + 3;
			} else {
				fprintf(ofp, " ");
				index++;
			}
		} else {
			fprintf(ofp, "%c", geofence[index]);
			index++;
		}

	}

	printf("hello world");
	fclose(ifp);
	fclose(ofp);


	return 1;
}
