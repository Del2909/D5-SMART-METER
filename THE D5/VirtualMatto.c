#include "VirtualMatto.h"
#include "stdio.h"

int triggerFlag = 0;


//*************************************************************
// STUDENT VARIABLES GO HERE
//*************************************************************

double temp, mains, oversupply, busbarvoltage;
double battery, cumulative_error, old_error;
double load1_switch, load2_switch, load3_switch, wind_renewable, pv_renewable, load1_call, load2_call, load3_call; // Creating a variable.
//NOTE: For simulation to work, use only "double" as the data type.
// for boolean values, use 1.0 to represent "True" and 0.0 to represent "False"
// end example
// ---------------------- 

//*************************************************************


void update(inputVector* input, double clk, outputVector* output)
{

	if (clk == 1 && triggerFlag == 0) //Only trigger at edge
	{
		triggerFlag = 1;
		//*************************************************************
		// STUDENT CODES GO HERE
		temp = 0;

		//Getting Input    
		load1_call = input->Di1 *0.8;
		load2_call = input->Di2*1.8;
		load3_call = input->Di3*1.4;
		wind_renewable = input->Ai3;
		pv_renewable = input->Ai4;
		busbarvoltage = input->Ai1;
		double error = busbarvoltage - 230;
		cumulative_error = cumulative_error + error;
		double differenceinerror = error - old_error;
		old_error = error;
		double total_renewable = wind_renewable + pv_renewable;
		double total_load = (load1_call + load2_call + load3_call) - ((error*0.365) + (cumulative_error*0.01) + ((differenceinerror / 0.1)*0.005));// +(cumulative_error*0.01) + ((differenceinerror / 0.1)*0.005));
		if (total_renewable >= total_load)                          //0.2,0.01,0.1(current_best)
		{														//0.8(too high),0.01,0.005
																//0.365
			double difference_without_mains = total_renewable - total_load;
			if (difference_without_mains >= 0)
			{

				if (difference_without_mains <= 1.0)
				{
					output->Ao2 = -difference_without_mains;//charging battery
					battery = battery + difference_without_mains;
					output->Ao1 = 0.0;
					output->Do1 = 1.0;
					output->Do2 = 1.0;
					output->Do3 = 1.0;
				}
				else
				{
					output->Ao2 = -1.0;//charging battery
					battery = battery + 1.0;
					output->Ao1 = 0.0;
					output->Do1 = 1.0;
					output->Do2 = 1.0;
					output->Do3 = 1.0;
				}
			}
			else {
				output->Ao1 = 0.0;
				output->Do1 = 1.0;
				output->Do2 = 1.0;
				output->Do3 = 1.0;
			}

		}
		else
		{
			double mains_needed = total_load - (total_renewable);//added oversupply here

			if ((mains_needed >= 0.0) && (mains_needed <= 2.0))
			{

				output->Ao1 = mains_needed + 1.0;
				output->Do1 = 1.0;
				output->Do2 = 1.0;
				output->Do3 = 1.0;
				output->Ao2 = -1.0;//charging battery
				battery = battery + 1.0;
			}

			else if ((mains_needed > 2.0) && (mains_needed <= 3.0)) {
				output->Do1 = 1.0;
				output->Do2 = 1.0;
				output->Do3 = 1.0;
				output->Ao1 = 3.0;//can still charge here with less than 1.0 but will leave it out first.
				double chargable = 3.0 - mains_needed;
				output->Ao2 = -chargable;//charging battery
				battery = battery + chargable;
			}

			else
			{
				//output->Ao1 = 3.0;
				double required_discharge = mains_needed - 3.0;

				if (battery >= required_discharge)
				{

					if (required_discharge <= 1.0)
					{

						output->Ao2 = +required_discharge;
						battery = battery - required_discharge;
						output->Do1 = 1.0;
						output->Do2 = 1.0;
						output->Do3 = 1.0;
						output->Ao1 = 3.0;
					}
					else
					{
						double emergency_power_phase_1 = load1_call + load2_call;
						if (total_renewable >= emergency_power_phase_1)
						{
							double emergency_charge = total_renewable - emergency_power_phase_1;
							if (emergency_charge <= 1.0)
							{
								output->Ao2 = -emergency_charge;
								battery = battery + emergency_charge;
								output->Ao1 = 0.0;
								output->Do1 = 1.0;
								output->Do2 = 1.0;
								output->Do3 = 0.0;
							}
							else
							{
								output->Ao2 = -1.0;
								battery = battery + 1.0;
								output->Ao1 = 0.0;
								output->Do1 = 1.0;
								output->Do2 = 1.0;
								output->Do3 = 0.0;
							}


						}
						else
						{
							if ((total_renewable + 3.0) >= emergency_power_phase_1)
							{
								double first_phase_mains_needed = emergency_power_phase_1 - total_renewable;
								if ((first_phase_mains_needed >= 0.0) && (first_phase_mains_needed <= 2.0))
								{

									output->Ao1 = first_phase_mains_needed + 1.0;
									output->Do1 = 1.0;
									output->Do2 = 1.0;
									output->Do3 = 0.0;
									output->Ao2 = -1.0;//charging battery
									battery = battery + 1.0;
								}
								else if ((first_phase_mains_needed > 2.0) && (first_phase_mains_needed <= 3.0))
								{
									output->Do1 = 1.0;
									output->Do2 = 1.0;
									output->Do3 = 0.0;
									output->Ao1 = 3.0;//can still charge here with less than 1.0 but will leave it out first.
									double chargable = 3.0 - first_phase_mains_needed;
									output->Ao2 = -chargable;//charging battery
									battery = battery + chargable;
								}
								else if (first_phase_mains_needed > 3.0)
								{
									double required_emergency_discharge_phase1 = first_phase_mains_needed - 3.0;
									if (required_emergency_discharge_phase1 <= 1.0)
									{
										output->Ao2 = +required_emergency_discharge_phase1;
										battery = battery - required_emergency_discharge_phase1;
										output->Do1 = 1.0;
										output->Do2 = 1.0;
										output->Do3 = 0.0;
										output->Ao1 = 3.0;
									}
									else
									{
										double emergency_power_phase_2 = load1_call;
										if (total_renewable >= emergency_power_phase_2)
										{
											double emergency_charge = total_renewable - emergency_power_phase_2;
											output->Ao2 = -emergency_charge;
											battery = battery + emergency_charge;
											output->Ao1 = 0.0;
											output->Do1 = 1.0;
											output->Do2 = 0.0;
											output->Do3 = 0.0;

										}
										else
										{
											if ((total_renewable + 3.0) >= emergency_power_phase_2)
											{
												double second_phase_mains_needed = emergency_power_phase_2 - total_renewable;
												if ((second_phase_mains_needed >= 0.0) && (second_phase_mains_needed <= 2.0))
												{

													output->Ao1 = second_phase_mains_needed + 1.0;
													output->Do1 = 1.0;
													output->Do2 = 0.0;
													output->Do3 = 0.0;
													output->Ao2 = -1.0;//charging battery
													battery = battery + 1.0;
												}
												else if ((second_phase_mains_needed > 2.0) && (second_phase_mains_needed <= 3.0))
												{
													output->Do1 = 1.0;
													output->Do2 = 0.0;
													output->Do3 = 0.0;
													output->Ao1 = 3.0;//can still charge here with less than 1.0 but will leave it out first.
													double chargable = 3.0 - second_phase_mains_needed;
													output->Ao2 = -chargable;//charging battery
													battery = battery + chargable;
												}
												else if (second_phase_mains_needed > 3.0)
												{
													double required_emergency_discharge_phase2 = second_phase_mains_needed - 3.0;
													if (battery >= required_emergency_discharge_phase2)
													{
														if (required_emergency_discharge_phase2 <= 1.0)
														{
															output->Ao2 = +required_emergency_discharge_phase2;
															battery = battery - required_emergency_discharge_phase2;
															output->Do1 = 1.0;
															output->Do2 = 0.0;//error here.
															output->Do3 = 0.0;
															output->Ao1 = 3.0;
														}
														else
														{
															output->Do1 = 0.0;
															output->Do2 = 0.0;
															output->Do3 = 0.0;
															output->Ao1 = 0.0;
														}


													}
													else
													{
														output->Do1 = 0.0;
														output->Do2 = 0.0;
														output->Do3 = 0.0;
														output->Ao1 = 0.0;

													}

												}
												else
												{
													output->Do1 = 0.0;
													output->Do2 = 0.0;
													output->Do3 = 0.0;
													output->Ao1 = 0.0;
												}
											}
											else
											{
												output->Do1 = 0.0;
												output->Do2 = 0.0;
												output->Do3 = 0.0;
												output->Ao1 = 0.0;
											}
										}
									}
								}
								else
								{
									output->Do1 = 1.0;
									output->Do2 = 1.0;
									output->Do3 = 0.0;
									output->Ao1 = first_phase_mains_needed;
								}
							}
							else
							{
								double first_phase_mains_needed = emergency_power_phase_1 - total_renewable;
								double required_emergency_discharge_phase1 = first_phase_mains_needed - 3.0;
								if (required_emergency_discharge_phase1 <= 1.0)
								{
									output->Ao2 = +required_emergency_discharge_phase1;
									battery = battery - required_emergency_discharge_phase1;
									output->Do1 = 1.0;
									output->Do2 = 1.0;
									output->Do3 = 0.0;
									output->Ao1 = 3.0;
								}
								else
								{
									double emergency_power_phase_2 = load1_call;
									if (total_renewable >= emergency_power_phase_2)
									{
										double emergency_charge = total_renewable - emergency_power_phase_2;
										output->Ao2 = -emergency_charge;
										battery = battery + emergency_charge;
										output->Ao1 = 0.0;
										output->Do1 = 1.0;
										output->Do2 = 0.0;
										output->Do3 = 0.0;

									}
									else
									{
										if ((total_renewable + 3.0) >= emergency_power_phase_2)
										{
											double second_phase_mains_needed = emergency_power_phase_2 - total_renewable;
											if ((second_phase_mains_needed >= 0.0) && (second_phase_mains_needed <= 2.0))
											{

												output->Ao1 = second_phase_mains_needed + 1.0;
												output->Do1 = 1.0;
												output->Do2 = 0.0;
												output->Do3 = 0.0;
												output->Ao2 = -1.0;//charging battery
												battery = battery + 1.0;
											}
											else if ((second_phase_mains_needed > 2.0) && (second_phase_mains_needed <= 3.0))
											{
												output->Do1 = 1.0;
												output->Do2 = 0.0;
												output->Do3 = 0.0;
												output->Ao1 = 3.0;//can still charge here with less than 1.0 but will leave it out first.
												double chargable = 3.0 - second_phase_mains_needed;
												output->Ao2 = -chargable;//charging battery
												battery = battery + chargable;
											}
											else if (second_phase_mains_needed > 3.0)
											{
												double required_emergency_discharge_phase2 = second_phase_mains_needed - 3.0;
												if (battery >= required_emergency_discharge_phase2)
												{
													if (required_emergency_discharge_phase2 <= 1.0)
													{
														output->Ao2 = +required_emergency_discharge_phase2;
														battery = battery - required_emergency_discharge_phase2;
														output->Do1 = 1.0;
														output->Do2 = 0.0;
														output->Do3 = 0.0;
														output->Ao1 = 3.0;
													}
													else
													{
														output->Do1 = 0.0;
														output->Do2 = 0.0;
														output->Do3 = 0.0;
														output->Ao1 = 0.0;
													}


												}
												else
												{
													output->Do1 = 0.0;
													output->Do2 = 0.0;
													output->Do3 = 0.0;
													output->Ao1 = 0.0;

												}

											}
											else
											{
												output->Do1 = 0.0;
												output->Do2 = 0.0;
												output->Do3 = 0.0;
												output->Ao1 = 0.0;
											}
										}
										else
										{
											output->Do1 = 0.0;
											output->Do2 = 0.0;
											output->Do3 = 0.0;
											output->Ao1 = 0.0;
										}
									}
								}
							}
						}
					}

				}
				else
				{//start

					double emergency_power_phase_1 = load1_call + load2_call;
					if (total_renewable >= emergency_power_phase_1)
					{
						double emergency_charge = total_renewable - emergency_power_phase_1;
						if (emergency_charge <= 1.0)
						{
							output->Ao2 = -emergency_charge;
							battery = battery + emergency_charge;
							output->Ao1 = 0.0;
							output->Do1 = 1.0;
							output->Do2 = 1.0;
							output->Do3 = 0.0;
						}
						else
						{
							output->Ao2 = -1.0;
							battery = battery + 1.0;
							output->Ao1 = 0.0;
							output->Do1 = 1.0;
							output->Do2 = 1.0;
							output->Do3 = 0.0;
						}


					}
					else
					{
						if ((total_renewable + 3.0) >= emergency_power_phase_1)
						{
							double first_phase_mains_needed = emergency_power_phase_1 - total_renewable;
							if ((first_phase_mains_needed >= 0.0) && (first_phase_mains_needed <= 2.0))
							{

								output->Ao1 = first_phase_mains_needed + 1.0;
								output->Do1 = 1.0;
								output->Do2 = 1.0;
								output->Do3 = 0.0;
								output->Ao2 = -1.0;//charging battery
								battery = battery + 1.0;
							}
							else if ((first_phase_mains_needed > 2.0) && (first_phase_mains_needed <= 3.0))
							{
								output->Do1 = 1.0;
								output->Do2 = 1.0;
								output->Do3 = 0.0;
								output->Ao1 = 3.0;//can still charge here with less than 1.0 but will leave it out first.
								double chargable = 3.0 - first_phase_mains_needed;
								output->Ao2 = -chargable;//charging battery
								battery = battery + chargable;
							}
							else if (first_phase_mains_needed > 3.0)
							{
								double required_emergency_discharge_phase1 = first_phase_mains_needed - 3.0;
								if (required_emergency_discharge_phase1 <= 1.0)
								{
									output->Ao2 = +required_emergency_discharge_phase1;
									battery = battery - required_emergency_discharge_phase1;
									output->Do1 = 1.0;
									output->Do2 = 1.0;
									output->Do3 = 0.0;
									output->Ao1 = 3.0;
								}
								else
								{
									double emergency_power_phase_2 = load1_call;
									if (total_renewable >= emergency_power_phase_2)
									{
										double emergency_charge = total_renewable - emergency_power_phase_2;
										output->Ao2 = -emergency_charge;
										battery = battery + emergency_charge;
										output->Ao1 = 0.0;
										output->Do1 = 1.0;
										output->Do2 = 0.0;
										output->Do3 = 0.0;

									}
									else
									{
										if ((total_renewable + 3.0) >= emergency_power_phase_2)
										{
											double second_phase_mains_needed = emergency_power_phase_2 - total_renewable;
											if ((second_phase_mains_needed >= 0.0) && (second_phase_mains_needed <= 2.0))
											{

												output->Ao1 = second_phase_mains_needed + 1.0;
												output->Do1 = 1.0;
												output->Do2 = 0.0;
												output->Do3 = 0.0;
												output->Ao2 = -1.0;//charging battery
												battery = battery + 1.0;
											}
											else if ((second_phase_mains_needed > 2.0) && (second_phase_mains_needed <= 3.0))
											{
												output->Do1 = 1.0;
												output->Do2 = 0.0;
												output->Do3 = 0.0;
												output->Ao1 = 3.0;//can still charge here with less than 1.0 but will leave it out first.
												double chargable = 3.0 - second_phase_mains_needed;
												output->Ao2 = -chargable;//charging battery
												battery = battery + chargable;
											}
											else if (second_phase_mains_needed > 3.0)
											{
												double required_emergency_discharge_phase2 = second_phase_mains_needed - 3.0;
												if (battery >= required_emergency_discharge_phase2)
												{
													if (required_emergency_discharge_phase2 <= 1.0)
													{
														output->Ao2 = +required_emergency_discharge_phase2;
														battery = battery - required_emergency_discharge_phase2;
														output->Do1 = 1.0;
														output->Do2 = 0.0;
														output->Do3 = 0.0;
														output->Ao1 = 3.0;
													}
													else
													{
														output->Do1 = 0.0;
														output->Do2 = 0.0;
														output->Do3 = 0.0;
														output->Ao1 = 0.0;
													}


												}
												else
												{
													output->Do1 = 0.0;
													output->Do2 = 0.0;
													output->Do3 = 0.0;
													output->Ao1 = 0.0;

												}

											}
											else
											{
												output->Do1 = 0.0;
												output->Do2 = 0.0;
												output->Do3 = 0.0;
												output->Ao1 = 0.0;
											}
										}
										else
										{
											output->Do1 = 0.0;
											output->Do2 = 0.0;
											output->Do3 = 0.0;
											output->Ao1 = 0.0;
										}
									}
								}
							}
							else
							{
								output->Do1 = 1.0;
								output->Do2 = 1.0;
								output->Do3 = 0.0;
								output->Ao1 = first_phase_mains_needed;
							}
						}
						else
						{
							double first_phase_mains_needed = emergency_power_phase_1 - total_renewable;
							double required_emergency_discharge_phase1 = first_phase_mains_needed - 3.0;
							if (required_emergency_discharge_phase1 <= 1.0)
							{
								output->Ao2 = +required_emergency_discharge_phase1;
								battery = battery - required_emergency_discharge_phase1;
								output->Do1 = 1.0;
								output->Do2 = 1.0;
								output->Do3 = 0.0;
								output->Ao1 = 3.0;
							}
							else
							{
								double emergency_power_phase_2 = load1_call;
								if (total_renewable >= emergency_power_phase_2)
								{
									double emergency_charge = total_renewable - emergency_power_phase_2;
									output->Ao2 = -emergency_charge;
									battery = battery + emergency_charge;
									output->Ao1 = 0.0;
									output->Do1 = 1.0;
									output->Do2 = 0.0;
									output->Do3 = 0.0;

								}
								else
								{
									if ((total_renewable + 3.0) >= emergency_power_phase_2)
									{
										double second_phase_mains_needed = emergency_power_phase_2 - total_renewable;
										if ((second_phase_mains_needed >= 0.0) && (second_phase_mains_needed <= 2.0))
										{

											output->Ao1 = second_phase_mains_needed + 1.0;
											output->Do1 = 1.0;
											output->Do2 = 0.0;
											output->Do3 = 0.0;
											output->Ao2 = -1.0;//charging battery
											battery = battery + 1.0;
										}
										else if ((second_phase_mains_needed > 2.0) && (second_phase_mains_needed <= 3.0))
										{
											output->Do1 = 1.0;
											output->Do2 = 0.0;
											output->Do3 = 0.0;
											output->Ao1 = 3.0;//can still charge here with less than 1.0 but will leave it out first.
											double chargable = 3.0 - second_phase_mains_needed;
											output->Ao2 = -chargable;//charging battery
											battery = battery + chargable;
										}
										else if (second_phase_mains_needed > 3.0)
										{
											double required_emergency_discharge_phase2 = second_phase_mains_needed - 3.0;
											if (battery >= required_emergency_discharge_phase2)
											{
												if (required_emergency_discharge_phase2 <= 1.0)
												{
													output->Ao2 = +required_emergency_discharge_phase2;
													battery = battery - required_emergency_discharge_phase2;
													output->Do1 = 1.0;
													output->Do2 = 0.0;
													output->Do3 = 0.0;
													output->Ao1 = 3.0;
												}
												else
												{
													output->Do1 = 0.0;
													output->Do2 = 0.0;
													output->Do3 = 0.0;
													output->Ao1 = 0.0;
												}


											}
											else
											{
												output->Do1 = 0.0;
												output->Do2 = 0.0;
												output->Do3 = 0.0;
												output->Ao1 = 0.0;

											}

										}
										else
										{
											output->Do1 = 0.0;
											output->Do2 = 0.0;
											output->Do3 = 0.0;
											output->Ao1 = 0.0;
										}
									}
									else
									{
										output->Do1 = 0.0;
										output->Do2 = 0.0;
										output->Do3 = 0.0;
										output->Ao1 = 0.0;
									}
								}
							}
						}
					}
					//end				
				}
			}
		}

		// STUDENT CODE END
//*************************************************************
	}
	else if (clk == 0)
	{
		triggerFlag = 0;
	}
	return output;

}