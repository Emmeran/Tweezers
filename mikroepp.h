// File: MikroEppendorf.h
// Description: interface for EppendorfManipuators
// Rev: A
// Created: 13. April 2005
// Author: Johannes Pauli
// mail: jpauli@johannes-pauli.de
//
// Copyright: GNU-GPL
//

#ifndef __MIKROEPPEN_H
#define __MIKROEPPEN_H

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>

// ctb needs this...
#define __WIN32__

// ctb headers
#include "wx\ctb\getopt.h"
#include "wx\ctb\serport.h"
#include "wx\ctb\timer.h"

// mm interface
#include "mikroman.h"

#include "El_timer.h"

using namespace std;

// send and receive buffersize
#define BUFFER_SIZE 256

class MikroEppendorf : public Mikromanipulator{
public:
	MikroEppendorf(char* comport = wxCOM1): Mikromanipulator(){

		// initialize buffers
		AnswerBuffer	= new char[BUFFER_SIZE];
		SendString		= new char[BUFFER_SIZE];

		// constants to change pix into mu-steps (depends on magnification)
		pix2mustep_x = 8;
		pix2mustep_y = 8;
		pix2mustep_z = 8;

		// constants for µm to steps (for Eppendorf InjectMan NI-2)
		mum2mustep_x = 25;
		mum2mustep_y = 25;
		mum2mustep_z = 25;

		// contstant distance to raise tip (100µm)
		TipUp_mustep = -100 * mum2mustep_z;

		// initialize device
		initDevice(comport);

		// activate remote control
		ActivateRemote();
	};

	~MikroEppendorf(){

		// switch back to hand control
		ActivateHandcontrol();

		// clean up
		if(com) {delete(com);};
		delete(SendString);
		//delete(AnswerBuffer);
	};

	virtual char* Version(void) const{
		return sendCommand("C001\r").get_Answer();
	};

protected:
	virtual Mikromanipulator& ResetDevice(void) const{
		return sendCommand("C002\r");
	};
	virtual Mikromanipulator& ResetMotors(void) const{
		return sendCommand("C003\r");
	};
	virtual Mikromanipulator& Halt(void) const{
		return sendCommand("C008\r");
	};

public:
	virtual Mikromanipulator& ActivateRemote(void) const{
		return sendCommand("C004\r");
	};
	virtual Mikromanipulator& ActivateHandcontrol() const{
		return sendCommand("C005\r");
	};

	virtual Mikromanipulator& gotoAbs_mustep(double x, double y, double z, double vx, double vy, double vz) const{
		sprintf(SendString,"C006 %.0f %.0f %.0f %.0f %.0f %.0f\r",x,y,z,vx,vy,vz);
		return sendCommand(SendString);
	};

	virtual Mikromanipulator& gotoAbs_mum(double x, double y, double z, double vx, double vy, double vz) const{
		sprintf(SendString,"C007 %.0f %.0f %.0f %.0f %.0f %.0f\r",x,y,z,vx,vy,vz);
		return sendCommand(SendString);
	};
	virtual Mikromanipulator& gotoAbs_mustep_fast(double x, double y, double z) const{
		sprintf(SendString,"C011 %.0f %.0f %.0f 192000 192000 192000\r",x,y,z);
		return sendOnlyCommand(SendString);
	};

	virtual Mikromanipulator& gotoAbs_mum_fast(double x, double y, double z) const{
		sprintf(SendString,"C012 %.0f %.0f %.0f 7500 7500 7500\r",x,y,z);
		return sendOnlyCommand(SendString);
	};

	virtual Mikromanipulator& gotoAbs_mustep_noWait(double x, double y, double z, double vx, double vy, double vz) const{
		sprintf(SendString,"C011 %.0f %.0f %.0f %.0f %.0f %.0f\r",x,y,z,vx,vy,vz);
		return sendCommand(SendString);
	};

	virtual Mikromanipulator& gotoAbs_mum_noWait(double x, double y, double z, double vx, double vy, double vz) const{
		sprintf(SendString,"C012 %.0f %.0f %.0f %.0f %.0f %.0f\r",x,y,z,vx,vy,vz);
		return sendCommand(SendString);
	};

	virtual unsigned int Position_mustep(double *x, double *y, double *z) const{
		sendCommand("C009\r");
		int i=0,j=0,k=0;
		char tmp[5];
		sscanf(AnswerBuffer,"%5s %d %d %d",tmp,&i,&j,&k);
		*x = (double)i;
		*y = (double)j;
		*z = (double)k;
		return 0;
	};

	virtual unsigned int Position_mum(double *x, double *y, double *z) const{
		sendCommand("C010\r");
		int i=0,j=0,k=0;
		char tmp[5];
		sscanf(AnswerBuffer,"%5s %d %d %d",tmp,&i,&j,&k);
		*x = (double)i;
		*y = (double)j;
		*z = (double)k;
		return 0;
	};

	virtual Mikromanipulator& StorePosition(unsigned int store = 0) const{
		sprintf(SendString,"C016 %d\r",store);
		return sendCommand(SendString);
	};

	virtual Mikromanipulator& RestorePosition(unsigned int store = 0) const{
		sprintf(SendString,"C017 %d\r",store);
		return sendCommand(SendString);
	};


	virtual Mikromanipulator& Wait4Motors(void) const{
		return sendCommand("C013\r");
	};

	inline Mikromanipulator& Beep(unsigned int num = 1) const{
		return BeepShort();
	};

	inline Mikromanipulator& BeepShort(unsigned int num = 1) const{
		sprintf(SendString,"C014 %2d\r",num);
		return sendCommand(SendString);
	};

	inline Mikromanipulator& BeepLong(unsigned int num = 1) const{
		sprintf(SendString,"C015 %2d\r",num);
		return sendCommand(SendString);
	};

protected:
	virtual Mikromanipulator& sendOnlyCommand(char* cmd) const {

		com->Write(cmd, strlen(cmd));

		AnswerBuffer[0] = '\0';

		return *((Mikromanipulator*)this);
	};

	virtual Mikromanipulator& sendCommand(char* cmd) const {

		int quit = 0;
	    timer t(200,&quit,NULL);
 
		t.start();

		char tmp[1];
		int read = 0;

		tmp[0] = 0;

//        while(!quit) com->Read(tmp, 1, &quit);
        while(com->Read(tmp, 1));

		quit = 0;

		tmp[0] = 0;

		//cerr << "Sending command: " << cmd << "\nlength: "<<strlen(cmd)<<"\n\n";

		com->Writev(cmd, strlen(cmd),0,1);

		while(tmp[0]!=13)
		{
		//	cerr<<"reading char "<<read+1;
            read += com->Readv(tmp, 1, &quit);
		//	cerr<<"...: "<<tmp[0]<<endl;
			AnswerBuffer[read-1] = tmp[0];
		}

	    if(quit) {
	       printf("timeout occured\n");
		}

		AnswerBuffer[read] = '\0';

		//cerr<<read<<" bytes received:"<<AnswerBuffer;

		//cerr<<"returning from send...\n";
		return *((Mikromanipulator*)this);
	};

	virtual Mikromanipulator& initDevice(char *devname = wxCOM1) const {

		// cast to make things more readable...
		MikroEppendorf* mm = (MikroEppendorf*)this;

		// create serial port object
		mm->com = new wxSerialPort;

		// parameters for Eppendorf: 19200-8N1
		mm->dcs.baud		= wxBAUD_19200;
		mm->dcs.wordlen		= 8;
		mm->dcs.parity		= wxPARITY_NONE;
		mm->dcs.stopbits	= 1;

		// open serial port
		if(com->Open(devname,&(mm->dcs)) < 0)
		{
		}

		return *mm;
	};

	// com port and settings
	wxSerialPort* com;
	wxSerialPort_DCS dcs;
};

#endif
