idf_component_register(
    SRCS "main.c" 
         "tasks/task_rx.c"
         "tasks/task_preprocess.c"
         "tasks/task_inference_eam.c"
         "tasks/task_inference_dem.c"
         "tasks/task_tx.c"
         "tasks/tasks.c"
    
    INCLUDE_DIRS "tasks"
    REQUIRES sigproc exp_approx_model dynamic_exp_model comm_protocol driver
)
