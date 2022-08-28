# LKM-Linux
It is a simple loadable kernel module which can handle multi-threading. Made it with references from already provided basic kernel module.
Main changes that are added are mutex locks and handling I/O. Also made a program for testing of the kernelModule.

First Run **make** command inside the terminal.
![image](https://user-images.githubusercontent.com/57556301/187073073-31f51bf6-9f1d-41c2-bc3d-e59b1de2c6a2.png)

After it look for .ko file.
![image](https://user-images.githubusercontent.com/57556301/187073106-b86d1b5e-6f74-46cc-bc3d-ea2081eccba4.png)

Run **insmod kernelDriver.ko** for insertion of kernel module inside kernel.

Use **tail -f /var/log/kern.log** command for monitoring the insertion of kenel module.

Now run Userspace.c.
