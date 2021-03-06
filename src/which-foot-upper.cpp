/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright Projet JRL-Japan, 2007
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      WhichFootUpper.h
 * Project:   SOT
 * Author:    Nicolas Mansard
 *
 * Version control
 * ===============
 *
 *  $Id$
 *
 * Description
 * ============
 *
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <sot-pattern-generator/which-foot-upper.h>
#include <sot/core/debug.hh>
#include <dynamic-graph/factory.h>
#include <sot/core/macros-signal.hh>
//#include <sot-pattern-generator/exception-pg.h>

namespace dynamicgraph {
  namespace sot {

    DYNAMICGRAPH_FACTORY_ENTITY_PLUGIN(WhichFootUpper,"WhichFootUpper");

    const unsigned int WhichFootUpper::
    INDEX_LEFT_FOOT_DEFAULT = 0;
    const unsigned int WhichFootUpper::
    INDEX_RIGHT_FOOT_DEFAULT = 1;

    const double WhichFootUpper::
    TRIGGER_THRESHOLD_DEFAULT = 5e-4;
    typedef Eigen::Matrix<double,4,4>& (MatrixHomogeneous::*ExtractMemberType) (void) const;

    WhichFootUpper::
    WhichFootUpper( const std::string & name )
      :Entity(name)
      ,indexLeftFoot( INDEX_LEFT_FOOT_DEFAULT )
      ,indexRightFoot( INDEX_RIGHT_FOOT_DEFAULT )
      ,triggerThreshold( TRIGGER_THRESHOLD_DEFAULT )
      ,lastFoot( indexLeftFoot )

      ,waistRsensorSIN( NULL,"WhichFootUpper("+name+")::input(matrixRotation)::waistRsensor" )
      ,worldRsensorSIN( NULL,"WhichFootUpper("+name+")::input(matrixRotation)::worldRsensor" )
      ,waistMlfootSIN( NULL,"WhichFootUpper("+name+")::input(matrixhomogeneous)::waistMlfoot" )
      ,waistMrfootSIN( NULL,"WhichFootUpper("+name+")::input(matrixhomogeneous)::waistMrfoot" )

      ,worldMlfootSOUT( SOT_INIT_SIGNAL_3( WhichFootUpper::computeFootPosition,
					   waistMlfootSIN,MatrixHomogeneous,
					   waistRsensorSIN,MatrixRotation,
					   worldRsensorSIN,MatrixRotation),
			"WhichFootUpper("+name+")::output(MatrixHomogeneous)::worldMlfoot" )
      ,worldMrfootSOUT( SOT_INIT_SIGNAL_3( WhichFootUpper::computeFootPosition,
					   waistMrfootSIN,MatrixHomogeneous,
					   waistRsensorSIN,MatrixRotation,
					   worldRsensorSIN,MatrixRotation),
			"WhichFootUpper("+name+")::output(MatrixHomogeneous)::worldMrfoot" )
      ,whichFootSOUT( SOT_MEMBER_SIGNAL_2( WhichFootUpper::whichFoot,
					   waistMlfootSIN,MatrixHomogeneous,
					   waistMrfootSIN,MatrixHomogeneous),
		      "WhichFootUpper("+name+")::output(uint)::whichFoot" )

      ,waistMsensorSIN( NULL,"WhichFootUpper("+name+")::input(matrixRotation)::waistMsensor" )
      ,waistRsensorSOUT( boost::bind(&WhichFootUpper::computeRotationMatrix,this,_1,_2),
			 waistMsensorSIN,
			 "WhichFootUpper("+name+")::output(MatrixHomogeneous)::waistRsensorOUT" )
    {
      sotDEBUGIN(5);
      signalRegistration( whichFootSOUT      << waistRsensorSIN
			  << worldRsensorSIN << waistMlfootSIN
			  << waistMrfootSIN  << worldMlfootSOUT
			  << worldMrfootSOUT
			  << waistMsensorSIN << waistRsensorSOUT);
      waistRsensorSIN.plug( &waistRsensorSOUT );
      sotDEBUGOUT(5);
    }




    WhichFootUpper::
    ~WhichFootUpper( void )
    {
      sotDEBUGINOUT(5);
      return;
    }

    /* --- SIGNALS -------------------------------------------------------------- */
    /* --- SIGNALS -------------------------------------------------------------- */
    /* --- SIGNALS -------------------------------------------------------------- */
    MatrixRotation& WhichFootUpper::
    computeRotationMatrix(MatrixRotation& rotMat, int time) {
      MatrixHomogeneous mh =  waistMsensorSIN(time);
      sotDEBUGIN(15);
      rotMat.resize(3,3);
      rotMat = mh.linear();
      return rotMat;
    }

    MatrixHomogeneous & WhichFootUpper::
    computeFootPosition( const MatrixHomogeneous& waistMfoot,
			 const MatrixRotation& waistRsensor,
			 const MatrixRotation& worldRsensor,
			 MatrixHomogeneous& worldMfoot )
    {
      sotDEBUGIN(15);

      MatrixRotation worldRwaist;
      worldRwaist = worldRsensor * waistRsensor.transpose();

      MatrixHomogeneous worldMwaist;
      worldMwaist.translation().setZero();
      worldMwaist.linear() = worldRwaist;

      worldMfoot = worldMwaist * waistMfoot;

      sotDEBUGOUT(15);
      return worldMfoot;
    }

    unsigned int & WhichFootUpper::
    whichFoot( const MatrixHomogeneous& waistMlfoot,
	       const MatrixHomogeneous& waistMrfoot,
	       unsigned int& res )
    {
      sotDEBUGIN(15);

      const double & leftAltitude = waistMlfoot(2,3);
      const double & rightAltitude = waistMrfoot(2,3);

      if( lastFoot==indexRightFoot )
	{
	  if( rightAltitude-triggerThreshold<leftAltitude )
	    { res = lastFoot; }
	  else { res = lastFoot = indexLeftFoot; }
	} else {
	if( leftAltitude-triggerThreshold<rightAltitude )
	  { res = lastFoot = indexLeftFoot; }
	else { res = lastFoot = indexRightFoot; }
      }

      sotDEBUGOUT(15);
      return res;
    }
  } // namespace dg
} // namespace sot
