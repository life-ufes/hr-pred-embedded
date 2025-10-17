idf_component_register(
    SRCS "main.c" 
         "tasks/task_receive.c"
         "tasks/task_preprocess.c"
         "tasks/task_inference.c"
         "tasks/task_send.c"
         "tasks/tasks.c"
    
    INCLUDE_DIRS "tasks"
    REQUIRES comm sigproc exp_approx_model driver
)
