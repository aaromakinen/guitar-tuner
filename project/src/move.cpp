/*
 * move.cpp
 *
 *  Created on: Oct 2, 2018
 *      Author: ASUS
 */
#include "move.h"


move::move() : xmove(0),ymove(0),x_currentcoor(0),
y_currentcoor(0),xleave_max(0),yleave_max(0),max_x(0),max_y(0),
dir_x(0,28,false,true,false),dir_y(1,0,false,true,false){

}

int move::desired_move(int x_newcoor, int y_newcoor){
	xmove = 0;
	ymove = 0;
	if (x_newcoor>37900){
		x_newcoor = 37900;
	}							// maybe change the max_x and y to not hit switches!
	if (y_newcoor>30900){
			y_newcoor = 30900;
		}
	x_newcoor = (x_newcoor * max_x) / 38000;

	y_newcoor = (y_newcoor * max_y) / 31000;
	delta.x = x_newcoor - x_currentcoor;
	delta.y = y_newcoor - y_currentcoor;
	if(delta.x % 2 != 0){
		delta.x++;
	}
	if(delta.y % 2 != 0){
		delta.y++;
	}



	xleave_max=abs(max_x - x_currentcoor);
	yleave_max=abs(max_y - y_currentcoor);

	if(delta.x >= 0){
		dir_x.write(true);
		dirx=true;
	}
	else{
		dir_x.write(false);
		dirx=false;
	}

	if(delta.y >= 0){
		dir_y.write(false);
		diry=false;
	}
	else{
		dir_y.write(true);
		diry=true;
	}


	/*if(abs(delta.x) >= xleave_max && dir_x.read() == false){
		x_currentcoor += xleave_max;

		delta.x = xleave_max;
	}
	else if( (abs(delta.x) >= x_currentcoor) && (dir_x.read() == true) ){
		x_currentcoor = 0;

		delta.x = x_currentcoor;			//maybe irrelevant !!!
	}
	else if(abs(delta.y) + y_currentcoor >= yleave_max && dir_y.read() == false){

		y_currentcoor += yleave_max;
		delta.y = yleave_max;
	}
	else if(abs(delta.y) >= y_currentcoor && dir_y.read() == true){
		y_currentcoor = 0;

		delta.y = y_currentcoor;

	}
	*/
	if(abs(delta.x) >= abs(delta.y)){

		ymove = abs(delta.x) - abs(delta.y);
		x_currentcoor += delta.x;
		y_currentcoor += delta.y;
		return abs(delta.x);

	}
	else{

		xmove = abs(delta.y) - abs(delta.x);
		y_currentcoor += delta.y;
		x_currentcoor += delta.x;
		return abs(delta.y);
	}
}


void move::change_dir(int axle,bool dir){
	if (axle == 0){
		dir_x.write(dir);
	}
	if (axle == 1){
		dir_y.write(dir);
	}
}




