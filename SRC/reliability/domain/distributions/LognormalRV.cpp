/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 2001, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** Reliability module developed by:                                   **
**   Terje Haukaas (haukaas@ce.berkeley.edu)                          **
**   Armen Der Kiureghian (adk@ce.berkeley.edu)                       **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.10 $
// $Date: 2008-04-10 16:23:32 $
// $Source: /usr/local/cvs/OpenSees/SRC/reliability/domain/distributions/LognormalRV.cpp,v $


//
// Written by Terje Haukaas (haukaas@ce.berkeley.edu)
//

#include <LognormalRV.h>
#include <NormalRV.h>
#include <cmath>
#include <Vector.h>

LognormalRV::LognormalRV(int passedTag, 
						 double passedMean, double passedStdv)
:RandomVariable(passedTag, RANDOM_VARIABLE_lognormal), startValue(0)
{
	if (passedMean<0.0) {
		isPositive = false;
		passedMean = -passedMean;
	}
	else {
		isPositive = true;
	}

	int setp = setParameters(passedMean,passedStdv);
	if (setp < 0)
		opserr << "Error setting parameters in Lognormal RV with tag " << this->getTag() << endln;
	
}


LognormalRV::LognormalRV(int passedTag, 
						 const Vector &passedParameters)
:RandomVariable(passedTag, RANDOM_VARIABLE_lognormal), startValue(0)
{
	if (passedParameters.Size() != 2) {
		opserr << "Lognormal RV requires 2 parameters, lambda and zeta, for RV with tag " <<
			this->getTag() << endln;
		
		// this will create terminal errors
		lambda = 0;
		zeta = 0;
		isPositive = true;
		
	} else {
		
		lambda = passedParameters(0);
		zeta = passedParameters(1);
		
		if (lambda<0.0) {
			isPositive = false;
			lambda = -lambda;
		}
		else {
			isPositive = true;
		}
	}
}


LognormalRV::~LognormalRV()
{
}


const char *
LognormalRV::getType()
{
	return "LOGNORMAL";
}


double 
LognormalRV::getMean()
{
	if (isPositive)
		return exp(lambda+0.5*zeta*zeta);
	else
		return -exp(lambda+0.5*zeta*zeta);
}


double 
LognormalRV::getStdv()
{
	return exp(lambda+0.5*zeta*zeta)*sqrt(exp(zeta*zeta)-1);
}


const Vector &
LognormalRV::getParameters(void) {
	static Vector temp(2);
	temp(0) = lambda;
	temp(1) = zeta;
	return temp;
}


int 
LognormalRV::setParameters(double mean, double stdv)
{
	zeta = sqrt(   log(   1+(stdv/mean)*(stdv/mean)   )   );
	lambda = log(mean) - 0.5*zeta*zeta;
	
	return 0;
}


double
LognormalRV::getPDFvalue(double rvValue)
{
	//static const double pi = std::acos(-1.0);
	
	if (!isPositive) {
		// The formal answer is: f(x) = f_pos(x+2|x|), but let's do it simple here
		rvValue = -rvValue;
	}

	double result;
	if ( 0.0 < rvValue ) {
		result = 1/(sqrt(2*pi)*zeta*rvValue) * exp(-0.5* pow ( (log(rvValue)-lambda) / zeta, 2 )  );
	}
	else {
		result = 0.0;
	}
	return result;
}


double
LognormalRV::getCDFvalue(double rvValue)
{
	double result;

	static NormalRV aStandardNormalRV( 1, 0.0, 1.0);
	
	if (isPositive) {
		if ( 0.0 < rvValue ) {
			result = aStandardNormalRV.getCDFvalue((log(rvValue)-lambda)/zeta);
		}
		else {
			result = 0.0;
		}
	}
	else {
		if ( rvValue < 0.0 ) {
			result = aStandardNormalRV.getCDFvalue((log(fabs(rvValue))-lambda)/zeta);
			result = 1.0-result;
		}
		else {
			result = 1.0;
		}
	}

	// Return result depending on type of random variable
	if (isPositive) {
		return result;
	}
	else {
		return 1-result;
	}



/*
	// First, flip it around if it's a negative lognormal
	if (!isPositive) {
		rvValue = -rvValue;
	}

	// Compute the ordinary CDF
	double result;
	if ( 0.0 < rvValue ) {
		RandomVariable *aStandardNormalRV;
		aStandardNormalRV= new NormalRV( 1, 0.0, 1.0, 0.0);
		result = aStandardNormalRV->getCDFvalue((log(rvValue)-lambda)/zeta);
		delete aStandardNormalRV;	
	}
	else {
		result = 0.0;
	}

	// Return result depending on type of random variable
	if (isPositive) {
		return result;
	}
	else {
		return 1-result;
	}
*/
}


double
LognormalRV::getInverseCDFvalue(double probValue)
{
	if ( probValue > 1.0 || probValue < 0.0) {
		opserr << "Invalid probability value input to inverse CDF function" << endln;
		return 0.0;
	}
	else {
		static NormalRV aStandardNormalRV( 1, 0.0, 1.0);

		if (isPositive) {
			double inverseNormal = aStandardNormalRV.getInverseCDFvalue(probValue);
			return exp(inverseNormal*zeta + lambda);
		}
		else {
			double inverseNormal = aStandardNormalRV.getInverseCDFvalue(1.0-probValue);
			return (-exp(inverseNormal*zeta + lambda));
		}
	}
}


void
LognormalRV::Print(OPS_Stream &s, int flag)
{
  s << "Lognormal RV #" << this->getTag() << endln;
  s << "\tlambda = " << lambda << endln;
  s << "\tzeta = " << zeta << endln;
}
