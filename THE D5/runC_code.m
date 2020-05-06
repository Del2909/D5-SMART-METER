%Virtual Arduino V1.4%
%Author: Ivan Ling%
%Last Update: 5 Apr 2020   3:28PM%
    
%***************************************************%
%First, initialize the legacy code import utility%
Simulink.importExternalCTypes('VirtualMatto.h');
def = legacy_code('initialize');
def.SourceFiles = {'VirtualMatto.c'};
def.HeaderFiles = {'VirtualMatto.h'};
%***************************************************%

%***************************************************%
%Map function prototype to MATLAB's prototype%
def.SFunctionName = 'VirtualMattoSim';
def.OutputFcnSpec = ['void update(inputVector u1[1], double u2, outputVector y1[1])'];
%  
%  Update note: 
%  V1.2: Double is used instead of int32 because the model generated
%  defaults to double.
%  V1.3: Updated to allow input bus.
%***************************************************%

%***************************************************%
%Generate SIMULINK Block to be copied into your simulation%
%legacy_code('sfcn_cmex_generate', def);
%legacy_code('compile', def);
legacy_code('generate_for_sim',def);
legacy_code('slblock_generate', def);
%***************************************************%