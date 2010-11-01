#include "LBPhysics.h"
#include <Logging/Logger.h>

#include <cmath>
//#include <fstream>

using namespace OpenEngine;
using Core::IListener;
using Core::InitializeEventArg;
using Core::ProcessEventArg;
using Resources::Texture3DChangedEventArg;


float omega = 1.0f;
float value = 0.3;
//int demoval = 1;
/*
//loggin
std::ofstream outdensity;
std::ofstream outvel;
std::ofstream velpostgrav;
std::ofstream rhoMax;
std::ofstream rhoMin;
std::ofstream rhoAverage;
std::ofstream vel_ux_max;
std::ofstream vel_ux_min;
std::ofstream vel_uy_max;
std::ofstream vel_uy_min;
std::ofstream vel_uz_max;
std::ofstream vel_uz_min;
std::ofstream FeqMax;
std::ofstream FeqMin;
std::ofstream BMax;
std::ofstream BMin;
*/
//bool debug = true;

//D3Q19

const int m = 19;
const int MY_SIZE = 50;

float cells[2][MY_SIZE][MY_SIZE][MY_SIZE][m];
unsigned char density[MY_SIZE*MY_SIZE*MY_SIZE*4];	
//unsigned char density[siz*MY_SIZE*MY_SIZE];
bool flag[MY_SIZE][MY_SIZE][MY_SIZE];
/*
bool NOSLIP = 1;
bool FLUID = 0;
*/
const float fracA = 0.33333333f;
const float fracB = 0.0555556f;
const float fracC = 0.0277778f;

const float w[19] = {fracA, fracB, fracB, fracB, fracB, fracB, fracB, 
					fracC, fracC, fracC, fracC, fracC, fracC, fracC, fracC, 
					fracC, fracC, fracC, fracC};

//const int ex[19] = {0,0,0,1,-1,0,0,1,-1,1,-1,0,0,0,0,1,1,-1,-1};
//const int ey[19] = {0,1,-1,0,0,0,0,1,1,-1,-1,1,1,-1,-1,0,0,0,0};
//const int ez[19] = {0,0,0,0,0,1,-1,0,0,0,0,1,-1,1,-1,1,-1,1,-1};
//const int finv[19] = {0,2,1,4,3,6,5,10,9,8,7,14,13,12,11,18,17,16,15};

const float ex[19] = {0.f,0.f,0.f,1.f,-1.f,0.f,0.f,1.f,-1.f,1.f,-1.f,0.f,0.f,0.f,0.f,1.f,1.f,-1.f,-1.f};
const float ey[19] = {0.f,1.f,-1.f,0.f,0.f,0.f,0.f,1.f,1.f,-1.f,-1.f,1.f,1.f,-1.f,-1.f,0.f,0.f,0.f,0.f};
const float ez[19] = {0.f,0.f,0.f,0.f,0.f,1.f,-1.f,0.f,0.f,0.f,0.f,1.f,-1.f,1.f,-1.f,1.f,-1.f,1.f,-1.f};
const float finv[19] = {0.f,2.f,1.f,4.f,3.f,6.f,5.f,10.f,9.f,8.f,7.f,14.f,13.f,12.f,11.f,18.f,17.f,16.f,15.f};


////setup cells, only one time.
/// setup grid boundary
/*
int round(float n) {
	if ( floor(n) == floor(n+0.5))
		return (int)floor(n);
	else
		return (int)ceil(n);
}
*/
//void maxMin(float&, float&, float&);

//Maybe this thin with old and new is a general error in the code, find out.

void LBPhysics::StreamStep(){
	for(int i = 1; i < MY_SIZE-1; i++){
		for(int j = 1; j < MY_SIZE-1; j++){
			for(int k = 1; k < MY_SIZE-1; k++){
				for(int l = 0; l < m; l++){
					int inv = finv[l];
                    cells[current][i][j][k][l] = 
                        cells
                        [other]
                        [i+(int)ex[inv]]
                        [j+(int)ey[inv]]
                        [k+(int)ez[inv]]
                        [l];
				}
			}
		}
	}
}

//const float minimum = ;
//const float maximum = 6.0;
//static unsigned int it = 0;

void LBPhysics::CollideStep() {
    /*
	if(debug && !outvel.is_open()){
		outdensity.open("densityvalue");
		outvel.open("velval");
		velpostgrav.open("velpostgrav");
		rhoMax.open("densityMax");
		rhoMin.open("densityMin");
		rhoAverage.open("densityAverage");
		vel_ux_max.open("vel_ux_max");
		vel_ux_min.open("vel_ux_min");
		vel_uy_max.open("vel_uy_max");
		vel_uy_min.open("vel_uy_min");
		vel_uz_max.open("vel_uz_max");
		vel_uz_min.open("vel_uz_min");
		FeqMax.open("feq_max");
		FeqMin.open("feq_min");
		BMax.open("b_max");
		BMin.open("b_min");

		outdensity.precision(6);
		//outvel.precision(6);
		//velpostgrav.precision(6);
	}
    */
    /*
	float minrho = 100000.f;
	float maxrho = -100000.f;
	float minFeq = 100000.f;
	float maxFeq = -100000.f;
	float minB = 100000.f;
	float maxB = -100000.f;

	float min_ux = 100000.f; 
	float max_ux = -100000.f; 
	float min_uy = 100000.f; 
	float max_uy = -100000.f; 
	float min_uz = 100000.f; 
	float max_uz = -100000.f; 

	float avrho = 0.f;
    */
	//std::cout << "updating density texture" << std::endl;
	for(int i = 1; i < MY_SIZE-1; i++){
		for(int j = 1; j < MY_SIZE-1; j++){
			for(int k = 1; k < MY_SIZE-1; k++){
				//calculate density and velocity
				float rho = 0.0f;
				float ux = 0.0f;
				float uy = 0.0f;
				float uz = 0.0f;
				for(int l = 0; l < m; l++){
					float fi = cells[current][i][j][k][l];
					rho += fi;

					ux += fi * ex[l];
					uy += fi * ey[l];
					uz += fi * ez[l];

				}
				//if(debug){
				//	if(i*2 == MY_SIZE && j*2 == MY_SIZE && k*2 == MY_SIZE)
				//	//if(i*2==MY_SIZE && j*2==MY_SIZE & k*2==MY_SIZE)
				//		outdensity << it << " "<< rho << std::endl;
				//}
				
				/*
				if(demoval == 1 && i*2==MY_SIZE && j*2==MY_SIZE && k*2== MY_SIZE && it%100==0) 
					uy += value;

				if(demoval == 2 && i==10 && j*2==MY_SIZE && k*2==MY_SIZE && it%100==0) //left side
					ux -= 0.3f;
				if(demoval == 2 && i == MY_SIZE-10 && j*2==MY_SIZE && k*2==MY_SIZE && it%100==0) //right side
					ux += 0.3f;
                */

				//this is the working one
				
				unsigned int width = MY_SIZE;
				unsigned int height = MY_SIZE;
				unsigned int entry = i + j * width + k * width * height;
					
				float densMax = 1.0f;
				float densMin = 0.50f;
				//unsigned char value = ceil( ( (rho/0.1f)-0.95f)*255);
				unsigned char value = ( ((rho* -1.0) - densMin) / (densMax - densMin) ) * 255; 
				density[(entry)*4+0] = 0; //red
				density[(entry)*4+1] = 0; //green
				density[(entry)*4+2] = 0; //blue
				if (rho > 1) { // greater than one
				density[(entry)*4+0] = 255; //red
				density[(entry)*4+1] = 200; //green
				density[(entry)*4+2] = 200; //blue
				} else { //lover meaning empty.
				density[(entry)*4+0] = 0; //red
				density[(entry)*4+1] = 0; //green
				density[(entry)*4+2] = 255; //blue
				}
				density[(entry)*4+3] = value; //alpha
				//density[(entry)*4+3] = rho*255.f; //alpha
				//std::cout  << it  << std::endl;
				//perform collision

				//float feq = 0.f;
				float b = 0.f;
				for(int l = 0; l < m; l++){
					float a = ex[l] * ux + ey[l] * uy + ez[l] * uz;
					float vel = ux*ux + uy*uy + uz*uz;

					float feq = w[l] * rho * (1.0 + (3.f * a) - 
						(3.f/2.f * vel) +
						(9.f/2.f * a*a));

					b = (1.0f - omega) * cells[current][i][j][k][l] + omega * feq;
						//float tmp = cells[current][i][j][k][l];
					//b = 0.f * cells[current][i][j][k][l] + omega * feq;
					cells[current][i][j][k][l] = b;

					//b = tmp;
				}
                /*
				if(debug){
					//if(i*2==MY_SIZE && j*2==MY_SIZE && k*2==MY_SIZE)
					//outvel << it << " " << b << std::endl;
					//}
					maxMin(rho, minrho, maxrho);
					maxMin(feq, minFeq, maxFeq);
					maxMin(b, minB, maxB);
					maxMin(ux, min_ux, max_ux);
					maxMin(uy, min_uy, max_uy);
					maxMin(uz, min_uz, max_uz);

					//average density
					avrho += rho;
				}
                */
			}
		}	
	}
    /*
	if(debug){
		rhoMin << it << " " << minrho << std::endl;
		rhoMax << it << " " << maxrho << std::endl;
		vel_ux_max << it << " " << max_ux << std::endl;
		vel_ux_min << it << " " << min_ux << std::endl;
		vel_uy_max << it << " " << max_uy << std::endl;
		vel_uy_min << it << " " << min_uy << std::endl;
		vel_uz_max << it << " " << max_uz << std::endl;
		vel_uz_min << it << " " << min_uz << std::endl;
		FeqMax << it << " " << maxFeq << std::endl;
		FeqMin << it << " " << minFeq << std::endl;
		BMax << it << " " << maxB << std::endl;
		BMin << it << " " << minB << std::endl;	

		//calculate average density
		float val = avrho / ((MY_SIZE-2)*(MY_SIZE-2)*(MY_SIZE-2));
		rhoAverage << it << " " << val << std::endl;
	}
    */

	//it++;
	//if(it==100)
		//exit(0);
}

void LBPhysics::Swap() {
	other = current;
	current = 1 - current;
}

/*
void Loggin(bool val){
	debug = val;	
}

void stopLog(){
	outdensity.close();
	outvel.close();
	velpostgrav.close();
	rhoMax.close();
	rhoMin.close();
	rhoAverage.close();
	vel_ux_max.close();
	vel_ux_min.close();
	vel_uy_max.close();
	vel_uy_min.close();
	vel_uz_max.close();
	vel_uz_min.close();
	FeqMax.close();
	FeqMin.close();
	BMax.close();
	BMin.close();
}
*/
/*
void maxMin(float& val, float& min, float& max){
	if(val < min)
		min = val;
	if(val > max)
		max = val;
}
int getCellSize(){
	return MY_SIZE;
}
*/

LBPhysics::LBPhysics() {
    data = density;
    width = MY_SIZE;
    height = MY_SIZE;
    depth = MY_SIZE;
    channels = 4;
    id = 0;
    format = Resources::RGBA;

    current = 0;
    other = 1;
}

LBPhysicsPtr LBPhysics::Create() {
    LBPhysicsPtr ptr(new LBPhysics());
    ptr->weak_this = ptr;
    return ptr;
}

// setupCells();
void LBPhysics::Handle(Core::InitializeEventArg arg) {
	for(int i = 0; i < MY_SIZE; i++) {
		for(int j = 0; j < MY_SIZE; j++) {
			for(int k = 0; k < MY_SIZE; k++) {
				// setup cells with DF
				for(int l = 0; l < m; l++) {
					cells[0][i][j][k][l] = w[l];
					cells[1][i][j][k][l] = w[l];
				}
			}
		}
	}
}

// runCellSystem();
void LBPhysics::Handle(Core::ProcessEventArg arg) {
	StreamStep();
	CollideStep();
	Swap();

    changedEvent
        .Notify(Texture3DChangedEventArg(Resources::ITexture3DPtr(weak_this)));
}
