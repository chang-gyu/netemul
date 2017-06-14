# PacketNgin NetEmul
PacketNgin NetEmul is Network Emulator, a tool for making virtualized network topology.

##Getting Started
PacketNgin NetEmul depends on PacketNgin libraries. however, you don't need to download PacketNgin library because it is already in the package of NetEmul.

If you have a problem during compiling OS, please visit PacketNgin website and follow install guide <http://packetngin.org>.
       
##Guides and Howtos

#####Download PacketNgin NetEmul & compile(for linux)

    $ git clone https://github.com/packetngin/netemul
    $ cd netemul
    $ ./premake5 gmake
    $ make
     
    
#####Execution guide

    $sudo ./netemul [OPTIONS]

* _OPTIONS_

> help : list available command and usage.
>> **help | -h**
>
> script : designate file path of script.
>> **script | -s [FILE]**
>
> version : show current version of Network Emulator.
>> **version | -v**

##Command Line Interface
To quit the Network Emulator.

    $ exit

To list the available commands and it's usage.

    $ help
    
To list information about nodes. If type of node is not specified, list up all node information.

    $ list [NODE_TYPE]

* _NODE_TYPE_

> Physical : Physical End point device. 
>> **host** | **-p**
>
> Host : Virtual End point device. 
>> **host** | **-p**
>
> Link : Connection link between two nodes.
>> **link** | **-l**
>
> Switch : Ethernet switch.
>> **switch**  | **-s**
>
> Hub : Ethernet dummy hub switch.
>> **hub** | **-h**
>
    
To create network nodes.

	$ create [NODE_TYPE] [OPTIONS]
    
More Specifically.

	$ create physical [PHYSICAL NETWORK INTERFACE NAME]
	$ create host [PORT_COUNT]
	$ create link [NODE] [NODE]
	$ create switch [PORT_COUNT]
	$ create hub [PORT_COUNT]
    
* _PORT_COUNT_

> Number of ports.
>> **1 ~ 64**
>
> It it is not given, default counts of specific node used.
>> Switch and Hub : **16**
>> Host : **4** 

* _NODE_

> Name of node. It will be automatically given like this.
>> Physical : **p0, p1, p2 ...**
>> Host     : **v0, v1, v2 ...**
>> Switch   : **s0, s1, s2 ...**
>> Hub      : **h0, h1, h1 ...**
>> Link     : **l0, l1, l2 ...**
>
> And also it can be specific port of node like this.
>> First port of host *v0* : **v0.0**
>> Second port of switch *s0* : **s0.1**

    
To destroy network nodes.
    
    $ destroy [NODE]
    
To activate network node.

    $ on [NODE]
    
To deactivate network node.

    $ off [NODE]
    
To get information of specific node. Information is diffrent by node type.

    $ get [NODE]
    
> Host and Hub switch : list of all ports and connected node in that port.
>
> Ethernet Switch : + MAC filtering table information.
>
> Link : two devices connected by this link.
   
To set attributes of specific node. 

    $ set [NODE] [OPTIONS]
    
Currently, this command is only for setting port of link. Users can use this command for controlling network like bandwidth limitation of link.

* _OPTIONS_

> Bandwidth - network bandwidth for unidirectional cable. it's bytes format.
>> **band: 0 ~ 100,000,000** 
> 
> Packet error rate - packet loss rate in a second. It's ratio format. 1 means 100% packet loss. 0 means packet loss never occured.
>> **error_rate: 0.0 ~ 1.0**
>
> Jitter - packet latency variance in a second.
>> **jitter: 0.0 ~ 1.0**
>
> Latency - latency of packet. It's milliseconds format. Could be set 1 second latency.
>> **latency: 0.0 ~ 1000.0**


##API
We provides API lists for developers : [Documentation on the NetEmul API](http://packetngin.org/assets/doxy/index.html "PacketNgin NetEmul API")

##Contact
Contact to our company by <contact@gurum.cc><br>
Inquiry about development by <changgyu@gurum.cc><br>
GurunmNetworks <http://gurum.cc><br>
PacketNgin OpenSource <http://packetngin.org><br>

##Licenses
PacketNgin NetEmul is licensed under GPL2
