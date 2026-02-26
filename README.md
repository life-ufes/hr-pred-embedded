# HR Predictor

### Requirements
It's highly recommended to install ESP-IDF VSCode extension. Follow the [official documentation](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/installation.html) instructions. It's also recommended to install `Microsoft C/C++ extension pack`.

### Usage
Firstly clone the repository and open VSCode within workspace directory. Then, open the command pallete and select:
```
> ESP-IDF: Add VS Code Configuration Folder
```
It resolves the compiler's path and syntax highlighting.

So, connect your device and:
```
> ESP-IDF: Select Port to Use 
```

Now you must select the device target:
```
> ESP-IDF: Set Espressif Device Target
```
 In this case the project is designed for ESP32S3, and OpenOCD Board Configuration uses `builtin USB-JTAG`.

At this point, the project is almost ready to run and you just have to build, flash and monitor selecting the following commands:

```
> ESP-IDF: Build Your Project
```

```
> ESP-IDF: Flash Your Project 
```
Select flash method. In this project I'm using `UART`.

:construction:
