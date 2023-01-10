# About

flbatuta~ is an audio external for Max/MSP written in C. For Windows only (for now). 'Batuta' is the spanish word for 'baton', the stick conductors use when directing a music ensamble. This external allows you to store and organize messages to play them in a chronological sequence similar to a music tracker.
In the same way a music score works, the external organizes time in bars, each bar has a time signature which divides the bar in beats, and the speed is controled by tempo. Repeat signs are called 'gotos' and have the same purpose. The bar is also divided in channels to identify every group of messages that will be called notes. The main differences with musical theory are that: 
- bars are measured from beat 0.0 instead of 1, much like a temporal ruler (also every indexation starts from 0) 
- the time signature isn't a ratio that uses different kinds of figures, it's just a fixed amount of beats that can also be a decimal number. i.e: 3.5 beats -> 7/8 time signature 
- the tempo is measured in time periods for each beat (in milliseconds) instead of a frequency. 500 ms -> 120 BPM.

This external works in 3 stages: editing, playing and loading. For the edit stage, the external uses linked lists to store and sort efficiently the different structs: bars, tempos, time signatures, gotos and notes. Then, an array of pointers is updated with the addresses of every struct to be accessed easily on the playing stage. Reading and writing a file are the loading stage.

<img src="examples/demobatuta.png">

Notice that the API says this about the t_linklist struct: "_A linklist element. This struct is provided for debugging convenience, but should be considered opaque and is subject to change without notice_". For this reason, using the 7.1.0 API version to build this project is highly recommended since its the only version that has been tested with. 

A compiled external (.mxe64) and a help file (.maxhelp) are provided in the example folder, as always.


### Inlets and Outlets

(From left to right)

**Inlets:**
- (int) on/off switch (play/edit)
- (int) next bar/set bar (play/edit) 

**Outlets:**
- (sig~) beat normalized ramp for syncing purposes
- (sig~) bar normalized ramp for syncing purposes
- (sig~) tempo
- (float) time signature
- (float) current bar index when playing
- (message) note channel and info
- (bang) end flag

<img src="examples/demobatuta_ramp.gif">

### Textfield commands

```
bars (bar, b)
	-n [new bar]
	-x [delete bar]
	-m (n) [move bar to (int)]
	-c (n) [copy bar and paste it in (int)]
channel (chan, ch)
	-w all / -w (n) / -w (min max) [show all/n/range channels]
	-e (n) / -e + (sel note ch) [edit channel to (int)]
	-x (n) / -x + (sel note ch) [delete whole channel]
	-c (n) / -c + (sel note ch) [copy channel to (int) bar]
	-h (n) (f) (r) / -h (f) (r) + (sel note ch) [changes +/- (f)ixed%% and (r)andom%% from beat start]
	-q (n) (d) / -q (d) + (sel note ch) [quantize to (d)ivision of beat (quarter note)]
notes (note, n)
	-s (ch) (n) [select a note from (n) index at (ch)an]
	-n (c) { (b) <(s) } (i) [new note with same (i)nfo to (s)ubdiv of (b)eat in (c)hannel]
		[example: -n 0 { 2.5 <101 1.5 <01 } bang
	-n (c) (s) (i) [new note with (i)nfo in (c)hannel at beat (s)tart]
	-e beat (s) + (sel note) [edit note beat (s)tart]
	-e chan (c) + (sel note) [edit note channel]
	-e info (i) + (sel note) [edit note info]
	-x + (sel note) [delete note]
time signature (tsign, ts)
	-n (b) [new time signature of n (b)eats per bar]
	-x [delete time signature]
go to (goto, gt)
	-n (n) (r) [go to bar (n) (r) times]
	-x [delete goto]
tempos (tempo, t)
	-n (p) [(p)eriod of beat]
	-n (p) (d) [new beat (p)eriod after (d)elay]
	-n (p) (d) (v) [change (p)eriod in (v)mseg starting after (d)elay]
	-n (p) (d) (v) (c) [idem but change with (c)urve]
	-x [delete tempo]
```

# Bug-fix/Features History

- MATLAB tempo curve file added
- Simplification of fl_tempo (tempo.type deleted, tempo.powval added)
- Duplicated channel when saving file bug fixed
- Storage in .JSON file using [json-c](https://github.com/json-c/json-c/wiki) (replaces storage in DATA file)
- Storage in .DATA file (replaces storage in text file)
- Note selection with mouse removed
- Info display on console corrected
- A default bar/tempo/time signature is added if element in first bar is deleted
- Comments and variables translated to english
- Note selection with mouse
- Texfield and command instructions added
- Storage in .txt file
- GUI added
- 'quantize' and 'human' functions added
- 'rec' function added
- t_atomarray replaced with t_atom dynamic memory allocation
- Error handling for 'out of memory' cases and linklist function error returns
- First commit














