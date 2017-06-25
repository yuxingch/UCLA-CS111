//	wk7

Follow Tutorial 5

https://iotdk.intel.com/docs/master/mraa/gpio_8h.html
mraa
gpio = mraa_gpio_init(3);


OUT -> IN

Read / Write

===

How to compute the temperature?

-lm


http://wiki.seeed.cc/Grove-Temperature_Sensor_V1.2/


MRAA CP
Temp CP

struct tm *localtime(const time_t *timep);
//	call time function with null argument
//	change the time zone

//	three threads
//	A
read(0, ...);
poll();
//	B
while {
	get temperature
	print/log
	sleep(T);
}
//	C: check for button

//	Respond to user's commands immediately
//	Doesn't waste any cycles
//	Drawbacks: each thread has its own stack. Cost more memory
