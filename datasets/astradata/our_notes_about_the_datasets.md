# Our notes about the files



## Dinghy

*It’s our gps transmitter which can record data locally of speed and Inertial platform status (pitch and roll), or transmit them via mobile and make them available to the user at any time. Those data can be playback form our iOS application.
the log contains date time stamp, id of the user, cog sog pitch and roll data.*

### Sample data
```
----------------------------- Device___15___2018-03-02 --------------------------
Date	Time	DinghyID	UserID	COG	SOG	LAT	LON	Pitch	Roll
2018/03/02	08:02:31	15	59cd04e5805f02002beba652	0.00	0.00	45.797419	12.817650	0.26	0.25

...
```

## Coach
*This is the data collected by our weather system placed on the coach boats. It is made by wind sensor gps and speed sensor and compass so we can calculate twd, tws awa aws tide in real time.*

*Those data are recordable by the iOS app that the coach uses like a dashboard during training.*

*Those data can also be merged like Jonas saw in the demo with dinghy’s data so for instance the user of a dinghy can see his data and the condition of the wind and tide or compare is data to others dinghy user.*

*In the folder you have 2 files, one is the complete data, the “charts” one is made explicit for doing charts, its’ deeply filtered to have a nice shape on the app, like you do with gps filtering etc.*


### Questions

  * Do you 

### Sample data

#### The raw file
```
----------------------------- log1Hz20180215_0957 --------------------------
----------------------------- DAMPING --------------------------
BS:	0.598
HDG:	0.598
AWA:	0.794
AWS:	0.696
COG:	0.596
SOG:	0.596
SET:	0.950
DRIFT:	0.950
TWA True:	0.950
TWS True:	0.956
TWD True:	0.965
TWA Ground:	0.900
TWS Ground:	0.900
TWD Ground:	0.900

----------------------------- LEEWAY & PROPELLER --------------------------
BS Point 1 - BS: 2.0	 GAIN: 1.300
BS Point 2 - BS: 6.9	 GAIN: 1.200
BS Point 3 - BS: 14.1	 GAIN: 1.000
BS Point 4 - BS: 28.2	 GAIN: 0.900

----------------------------- DONGLE SETTINGS --------------------------
WIND SENSOR:	ST-60/70 Raymarine
WIND OFFSET:	-175
TWA UPWIND:	0.0,0.0,0.0,0.0,0.0,0.0
TWA REACHING:	0.0,0.0,0.0,0.0,0.0,0.0
TWA DOWNWIND:	0.0,0.0,0.0,0.0,0.0,0.0
TWS CORR Kt:	0.0,0.0,0.0,0.0,0.0,0.0
TWS CORR Angle:	140.0,140.0,140.0,140.0,140.0,140.0
BS SENSOR:	NMEA
DEPTH:	0.20
HDG SENSOR:	NMEA
HDG OFFSET:	2

----------------------------- LOG --------------------------
Date	Time	Ts	BS	AWA	AWS	HDG	TWA	TWS	TWD	WindType	SOG	COG	Lat	Lon	Set	Drift	FreezeWind	FreezeTide
15/02/18	10:34:58	2614	0.15	144	3.91	280	145	4.03	65	True	0.03	273	4533.0557N	1338.5131E	-	-	NO	NO
15/02/18	10:34:59	2615	0.14	141	3.98	281	142	4.09	63	True	0.02	272	4533.0557N	1338.5131E	-	-	NO	NO

...
```

#### The ```_Chars``` suffixed file
```
----------------------------- log1Hz20180215_0957_Charts --------------------------
Date	Time	Ts	GWD	GWS	TWD	TWS	SET	DRIFT	Lat	Lon
15/02/18	10:34:58	2614	60	4.55	62	5.71	-	-	4533.0557N	1338.5131E
15/02/18	10:34:59	2615	60	4.50	62	5.63	-	-	4533.0557N	1338.5131E

...
```


## ESA Regata (regata/ subdirectory)
SECOND	COUNTDOWN	LON_BUOY_P	LAT_BUOY_P	LON_BUOY_S	LAT_BUOY_S	TWD	LON_BOAT	LAT_BOAT	BOAT_SPEED	DISTANCE_START_LINE	DISTANCE_P	DISTANCE_S	FL_TWD	GAIN	TTB_P	TTB_S	TTB_VMG_P	TTB_VMG_S	DISTANCE_VMG_P	DISTANCE_VMG_S	VMG	HEADING
20800	5:00	0.000000	0.000000	1341.052979	4540.818848	215.000000	1341.108643	4540.820312	6.400000	68.481194	5270292.500000	72.086647	285.433807	-4936773.500000	496:07:06	0:19	-:-	0:22	-256.725189	82.322739	7.200000	296.100006
