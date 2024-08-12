
load('./dof14.mat')
run("./driver.m")
run('./engine.m')
run('./cartype.m')
run('./driveline_para.m')
run('./moter_paramters.m')
run('./batt_paramters.m')
run('./ecu_paramters.m')
run('./hev_parameter.m')

load('./model_bus_define.mat')
load('./mdl_cfg_codegen.mat')

load('./dof14.mat', 'VEH')
