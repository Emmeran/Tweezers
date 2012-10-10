// File: Mikromanipulator.h
// Description: abstract interface definition for all Mikromanipulators
// Rev: A
// Created: 13. April 2005
// Author: Johannes Pauli
// mail: jpauli@johannes-pauli.de
//
// Copyright: GNU-GPL
//

#ifndef __MIKROMANIPULATOR_H
#define __MIKROMANIPULATOR_H

using namespace std;

class Mikromanipulator{
 public:
	 Mikromanipulator(){}
    //initDevice("wxCOM3").ActivateRemote();
  //};

	 virtual ~Mikromanipulator(){}
    //ActivateHandcontrol();
  //}

  virtual char* Version(void) const = 0;
 protected:
  virtual Mikromanipulator& ResetDevice(void) const = 0;
  virtual Mikromanipulator& ResetMotors(void) const = 0;
  virtual Mikromanipulator& Halt(void) const = 0;

 public:
  virtual Mikromanipulator& ActivateRemote(void) const = 0;
  virtual Mikromanipulator& ActivateHandcontrol() const = 0;

  virtual Mikromanipulator& gotoAbs_mustep(double x, double y, double z, double vx, double vy, double vz) const = 0;
  virtual Mikromanipulator& gotoAbs_mum(double x, double y, double z, double vx, double vy, double vz) const = 0;

  virtual Mikromanipulator& gotoAbs_mustep_noWait(double x, double y, double z, double vx, double vy, double vz) const = 0;
  virtual Mikromanipulator& gotoAbs_mum_noWait(double x, double y, double z, double vx, double vy, double vz) const = 0;
  
  virtual unsigned int Position_mustep(double *x, double *y, double *z) const = 0;
  virtual unsigned int Position_mum(double *x, double *y, double *z) const = 0;
  
  virtual Mikromanipulator& StorePosition(unsigned int store = 0) const = 0;
  virtual Mikromanipulator& RestorePosition(unsigned int store = 0) const = 0;

  virtual Mikromanipulator& Wait4Motors(void) const = 0;
  
  char* get_Answer() const{return AnswerBuffer;};

  //// Alias
  // gotoDirectAbs
  inline Mikromanipulator& gotoDirectAbs(double x, double y, double z, double vx, double vy, double vz) const{
    return gotoDirectAbs_mustep(x, y, z, vx, vy, vz);
  };
  inline Mikromanipulator& gotoDirectAbs_mustep(double x, double y, double z, double vx, double vy, double vz) const{
    return gotoAbs_mustep(x, y, z, vx, vy, vz);
  };
  inline Mikromanipulator& gotoDirectAbs_mum(double x, double y, double z, double vx, double vy, double vz) const{
    return gotoAbs_mum(x, y, z, vx, vy, vz);
  };
  inline Mikromanipulator& gotoDirectAbs_mustep_nowait(double x, double y, double z, double vx, double vy, double vz) const{
    return gotoAbs_mustep_noWait(x, y, z, vx, vy, vz);
  };
  inline Mikromanipulator& gotoDirectAbs_mum_nowait(double x, double y, double z, double vx, double vy, double vz) const{
    return gotoAbs_mum_noWait(x, y, z, vx, vy, vz);
  };

  // gotoDirectRel
  inline Mikromanipulator& gotoDirectRel(double x, double y, double z, double vx, double vy, double vz) const{
    return gotoDirectRel_mustep(x, y, z, vx, vy, vz);
  };
  inline Mikromanipulator& gotoDirectRel_mustep(double x, double y, double z, double vx, double vy, double vz) const{
    double px=0, py=0, pz=0;
    Position_mustep(&px, &py, &pz);
	x += px;
    y += py;
    z += pz;
	return gotoDirectAbs_mustep(x, y, z, vx, vy, vz);
  };
  inline Mikromanipulator& gotoDirectRel_mum(double x, double y, double z, double vx, double vy, double vz) const{
    double px=0, py=0, pz=0;
    Position_mum(&px, &py, &pz);
    x += px;
    y += py;
    z += pz;
    return gotoDirectAbs_mum(x, y, z, vx, vy, vz);
  };

  // no wait
  inline Mikromanipulator& gotoDirectRel_mustep_nowait(double x, double y, double z, double vx, double vy, double vz) const{
    double px=0, py=0, pz=0;
    Position_mustep(&px, &py, &pz);
	x += px;
    y += py;
    z += pz;
	return gotoDirectAbs_mustep_nowait(x, y, z, vx, vy, vz);
  };
  inline Mikromanipulator& gotoDirectRel_mum_nowait(double x, double y, double z, double vx, double vy, double vz) const{
    double px=0, py=0, pz=0;
    Position_mum(&px, &py, &pz);
    x += px;
    y += py;
    z += pz;
    return gotoDirectAbs_mum_nowait(x, y, z, vx, vy, vz);
  };


  // gotoTipUpAbs
  inline Mikromanipulator& gotoTipUpAbs(double x, double y, double z, double vx, double vy, double vz) const{
    return gotoTipUpAbs_mustep(x, y, z, vx, vy, vz);
  };
  inline Mikromanipulator& gotoTipUpAbs_mustep(double x, double y, double z, double vx, double vy, double vz) const{
    return TipUp(vz).gotoDirectAbs_mustep(x, y, z, vx, vy, 0).TipDown(vz);
  };
  inline Mikromanipulator& gotoTipUpAbs_mum(double x, double y, double z, double vx, double vy, double vz) const{
    return TipUp(vz).gotoDirectAbs_mum(x, y, z, vx, vy, 0).TipDown(vz);
  };

  // gotoTipUpRel
  inline Mikromanipulator& gotoTipUpRel(double x, double y, double z, double vx, double vy, double vz) const{
    return gotoTipUpRel_mustep(x, y, z, vx, vy, vz);
  };
  inline Mikromanipulator& gotoTipUpRel_mustep(double x, double y, double z, double vx, double vy, double vz) const{
    return TipUp(vz).gotoDirectRel_mustep(x, y, z, vx, vy, 0).TipDown(vz);
  };
  inline Mikromanipulator& gotoTipUpRel_mum(double x, double y, double z, double vx, double vy, double vz) const{
    return TipUp(vz).gotoDirectRel_mum(x, y, z, vx, vy, 0).TipDown(vz);
  };

 protected:
  inline Mikromanipulator& TipUp(double vz) const{
    return gotoDirectRel(0, 0, TipUp_mustep, 0, 0, vz);
  };
  Mikromanipulator& TipDown(double vz) const{
    return gotoDirectRel(0, 0, -TipUp_mustep, 0, 0, vz);
  };
    
inline Mikromanipulator& pix2mustep(double px, double py, double pz, double &mx, double &my, double &mz)const{
    mx = px * pix2mustep_x;
    my = py * pix2mustep_y;
    mz = pz * pix2mustep_z;

    return *((Mikromanipulator*)this);
  };
inline Mikromanipulator& pix2mustep(double &x, double &y, double &z) const{
    return pix2mustep(x, y, z, x, y, z);
  };

inline Mikromanipulator& mum2mustep(double px, double py, double pz, double &mx, double &my, double &mz)const{
    mx = px * mum2mustep_x;
    my = py * mum2mustep_y;
    mz = pz * mum2mustep_z;

    return *((Mikromanipulator*)this);
  };
inline Mikromanipulator& mum2mustep(double &x, double &y, double &z) const{
    return mum2mustep(x, y, z, x, y, z);
  };

 protected:
  virtual Mikromanipulator& sendCommand(char* cmd) const = 0;
  virtual Mikromanipulator& sendOnlyCommand(char* cmd) const = 0;
  virtual Mikromanipulator& initDevice(char *devname) const = 0;
  
  double pix2mustep_x;
  double pix2mustep_y;
  double pix2mustep_z;

  double mum2mustep_x;
  double mum2mustep_y;
  double mum2mustep_z;

  double TipUp_mustep;

  //char AnswerBuffer[256]; //!!! besser über schalter! ...
  char* AnswerBuffer; //!!! besser über schalter! ...
  char* SendString;
};

#endif
