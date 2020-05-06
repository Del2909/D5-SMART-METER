#include "VirtualMatto.h"

int triggerFlag = 0;

//*************************************************************
// STUDENT VARIABLES GO HERE
// Authors: Aathira, Saravanan and Dr. Ivan
//*************************************************************

//input variables
double busbarv;
double busbari;
double wind;
double photov;
double call1;
double call2;
double call3;

//control variables
double load_demand = 0.0;       	 //total load requirement
double supply_demand = 0.0;	    	//tally of power to be supplied
double rs = 0.0;					//renewable sources available
double rs_excess = 0.0;				//excess of renewable sources
double battery = 0.0;				//reading of momentary battery capacity
double pkcapacity = 0.0;			//availability of input power
double totalexcess = 0.0;			//power source excess available 
double loadusage = 0.0;				//current power use for load

//output variables	
double max_mains=3.0;				//sets the maximum power mains distribute
double mains = 0.0;					//draw from mains
double battout = 0.0;				//battery charge/discharge power
double load1 = 0.0;					//load1 switch 
double load2 = 0.0;					//load2 switch 
double load3 = 0.0;					//load3 switch
double over = 0.0;					//quantity exceeded by mains
double battpk =0.0;					//specifies the max discarge ability of battery					 
double p_err =0.0;					//error in busbar power
double PID=2.0;						//PID setting (1.0 is tuning mains only) and (2.0 is tuning battery only) 
double kp=52;						//P constant for mains tuning
double prev_err=0.0;				//previous busbar power error					
double t_mains=0.0;					//tuned mains output
double kp_b=82;						//P constant for tuning battery 
double t_batt=0.0;					//tuned battery output

//NOTE: For simulation to work, use only "double" as the data type.
// for boolean values, use 1.0 to represent "True" and 0.0 to represent "False"

//*************************************************************
void update(inputVector* input, double clk, outputVector* output)
{

	if (clk == 1 && triggerFlag == 0) //Only trigger at edge
	{
		triggerFlag = 1;
		//*************************************************************
		
		//reading inputs
		busbarv = input->Ai1; //busbar voltage
		busbari = input->Ai2; //busbar current
		wind = input->Ai3;    //busbar current
		photov = input->Ai4;  //busbar current
		
		//load call inputs
		call1 = input->Di1; //load 1 call
		call2 = input->Di2; //load 2 call
		call3 = input->Di3; //load 3 call
		
		//reinitialising variables
		load_demand  = 0.0;
		supply_demand = 0.0;
		battout = 0.0;
		mains = 0.0;	
		loadusage = 0.0;
		
		//compute max output of battery
		if (battery>=1.0) battpk=1.0;
		else battpk=battery;
		
		
		rs = wind + photov;					  		//total rs available
		pkcapacity = rs + max_mains + battpk ;      //total power available 
		totalexcess = pkcapacity;             		//excess availability after providing load
		
		//load priority
		if (call1 && (totalexcess >= 0.8))
		{	
			load1 = 1.0;
			totalexcess -= 0.8; 
		}	
		else load1 = 0.0;
		
		if (call2 && (totalexcess >= 1.8))
		{	
			load2 = 1.0;
			totalexcess -= 1.8;
		}
		else load2 = 0.0;
		
 		if (call3 && (totalexcess >= 1.4) ) 
		{	
			load3 = 1.0;
			totalexcess -= 1.4; 
		}
		else load3 = 0.0; 
		
		//computes required power
		loadusage = (load1*0.8) + (load2*1.8) + (load3*1.4);
		supply_demand = loadusage;
		
		//supply source management
		if (rs >= supply_demand) rs_excess = rs - supply_demand;
		
		else //if rs isnt enough
		{
			//maximize rs
			supply_demand -= rs; //use rs
			rs_excess = 0.0;
			
			//battery outputs
			if (battery>0.0)// if batt not enough
			{
				if (battery > 2.0) //if battery has excess
				{	
					if (battery >= supply_demand) 
					{ 
						if (supply_demand >= 1.0)
						{
							battout = 1.0;
//del							//battery -= 1.0; 
							supply_demand -= 1.0;
						}
						else 
						{
							battout = supply_demand;
//del							//battery -= supply_demand;
							supply_demand -= battery;
						} 
					}
				
					else //batt not enough provided that supply_demand< battery
					{	
						if (battery < 1.0)
						{
							battout = battery;; //discharge everything
							supply_demand -= battout;
//del							//battery = 0;
						}
					
						else 
						{
							battout = 1.0;
							supply_demand -= 1.0;
//del							//battery -= 1.0;
						}
					
						if (supply_demand <= max_mains) 
						{
							mains = supply_demand;
							supply_demand -= mains;
						}
				
						else 
						{
							mains = max_mains;	//overdemand - use load priority
							supply_demand -= mains;
						}
					}	
				}
				else if (battery<2.0 && supply_demand>max_mains) //if need more than 3.0 where mains won't suffice
				{
					mains=max_mains;
					supply_demand -=mains;
					battout= supply_demand;
					supply_demand-=battout;
//del					//battery-=battout;
				}
			
			}	
			//mains (use if batts empty)
			else 
			{
				if (supply_demand <= max_mains) 
				{
					mains = supply_demand;
					supply_demand -= mains;
				}
				
				else 
				{
					mains = max_mains;			//overdemand - use load priority```			//2.0
					supply_demand -= mains;
				}
			}
		}
		
		//Charging statements
		if (battout <= 0) 	//if not discharging
		{ 
			if (battery < 2.0)
			{
				if (rs_excess <= 1.0)			//charge at maximum rate by drawing from mains
				{
					mains += 1.0 - rs_excess;	//max mains can be increased by is 1.0
					battout = -1.0;
				}
	
				else battout=-1.0;				//if rs is greater than 1.0
				
				if (mains > max_mains) 			    //if the mains exceeded the 3.0 from previous case
				{
					over = mains - max_mains;			//over is positive integer less than 1.0
					mains = max_mains;
					battout = over - 1.0;		
				}
				
			} 
			
			else 
			{
				if (rs_excess <= 1.0) battout = rs_excess * (-1);
				else battout = -1.0;
			}
			
			//battery update
//del			//battery -= battout;
		}
		
		
		//tuning Mains and Battery
		if (PID>=1.0)
		{
		//mains PID tuning
		p_err=((230.0 -busbarv)*busbari)*(3.0/100000.0);
		t_mains= mains +(kp*p_err);
		if (t_mains<0) t_mains=0.0;
		if (t_mains>3.0) t_mains=3.0;
		}
		else t_mains=mains;
		
		if (PID>=2.0)
		{
		//battery PID tuning
		t_batt= battout + (kp_b*p_err);
		if (t_batt<-1.0) t_batt=-1.0;
		if (t_batt>1.0) t_batt=1.0;
		prev_err=p_err;
		}
		
		else t_batt=battout;
		
		battery -=battout;
		
		
		//load outputs
        output->Do1 = load1 ; //load 1
        output->Do2 = load2 ; //load 2
        output->Do3 = load3 ; //load 3
		
		//source outputs
        output->Ao1 = t_mains ; //how much to take from mains
        output->Ao2 = t_batt ; //battery control
		// STUDENT CODE END
		//*************************************************************

	}
	else if (clk == 0)
	{
		triggerFlag = 0;
	}
	return output;
}
