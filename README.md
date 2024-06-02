How to BUILD

1. Install the yocto building system on your local machine; 
There are multiple sources which describe how to do it ; 
I myself used this one: https://www.youtube.com/watch?v=5hgeAuQ454U
The finishing step should be perfroming the command : 
bitbake core-image-minimal ; This will build the minumum image - qemux86 machine; 

2. After you firstly build the image, you should do one specific thing which then be used for the purposes of interaction with
kernel module itself; 
    1. Go to yocto/poky/build/conf 
    2. Edit the local.conf file : by adding the openssh feature int the file
    3. modify EXTRA_IMAGE_FEATURES ?= "debug-tweaks" into-> EXTRA_IMAGE_FEATURES ?= "debug-tweaks ssh-server-openssh"

3. Then depending you need to create and add a meta-layer:
    3.1 Create a new meta-layer named watchpoint
    3.2 Add a new layer (after adding it shoould be reflected in conf/bblayers.conf file) 

4. After doing above steps , you would have new layer created : yocto/poky/meta-watchpoint; on the level poky/ there would be all the repositories;
5. Go inside , create recipes-kernel folder; mkdir recipes-kernel
6. Create watchpoint folder; mkdir watchpoint
7. Here install the files from this filder;
8. After all these long installation steps go back to yocto/poky/build and run command bitbake watchpoint ; This will build the watchpoint.ko kernel module;

How to Test
1. Now , in order to test it we need to install and ryn our module; 
To do this the first step is - runqemu ; (dont forget about that step 2 of the build );
2. At this point you will a new window opening which is and image iteslf; 
3. When machine finsihes it starting up procees the login would be required; On different sites its written that such a thing can happen ; 
my solution to this is a password : "root"; 

These steps above are the setup of qemux86; 
Now we need to go back to the host and open a new shell; we will use thi new windows 
in order to copy the built watchpoint.ko module into the qemux86 /tmp folder; 
this can be done by using ssh command 
ssh /path/to/watchpoint.ko root@192.168.7.2:/tmp 
After copying the file , return back to the qemu machine; 

In order to install the module 
 1. insmod watchpoint.ko 
 The user should see the output  : 
    module loading with watch_adress 0 ; 
This is by dy dafult the system inits the static var to 0 ;

After installing it as required by the task, you should be able to cahnge its adress using sysfs
so you need to go to wehere it lays : /sys/kernel/watch_module directory inside which you will find the gWatchMemAdress; 
This is essanetially our mem adress 

To watch its value -> cat ggWatchMemAdress 
You would get : The memory adress watched : o

To modify the value -> echo MEMORYADDRES > gWatchMemAdress ; 
You would see smth like : Reqeust to change ...
                          The new memory adress : MEMORYADDRES
                          Previous hardware breakpoint unregitered
                          Hardware breakppint registered to adress MEMORYADDRES

Here is the thing - as you may find out l not every number is a memory adress ; So if you type 12345678 this may result in 
    Failed to register hardawe breakpoint; 
Thats because you such memadress doenst exist;

What i did to test my watchpoint was to create another kernel module which inside contains a simple int variable(for this repeate all the steps above for memaccess)
In that module i expose the memory adress of that variable and then ++ its value ;
The module is called memaccess; and you can find it this repository and set up in the same meta-layer; 

I then use this adress in my watchpoint module; 
echo db12345 > ggWatchMemAdress

After that go to the sys/kerne;/memaccess 
and type : cat memaccess ;

Here you go ! YOu would see all the debug output and call trace !

The scipts i used on order to save time , you might do so as well;