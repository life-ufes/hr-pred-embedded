idf_component_register(
    SRCS "main.c" 
         "tasks/task_receive.c"
         "tasks/task_preprocess.c"
         "tasks/task_inference.c"
         "tasks/task_send.c"
    
    INCLUDE_DIRS "tasks"
)
