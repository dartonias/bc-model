// g++ -Wall -O3 `pkg-config --cflags --libs gsl tabdatrw interp2dpp` Mutual-info-vs-beta.cc -o mi
// 2nd Renyi entropy for classical 2d Ising model in zero magnetic field
//Metropolis algorithm employed
//Parameters that can be changed for different runs:
//J, axis1, axis2, N_mc

// S_2 (A)= 2 ln(Z[T])-ln(Z[A,2,T])
// S_2 (B)= 2 ln(Z[T])-ln(Z[B,2,T])
// S_2 (A U B)= 2 ln(Z[T])-ln(Z[T/2])
// I_2 (A; B) = S_2 (A) + S_2 (B) - S_2 (A U B) 
// = 2 ln(Z[T])+ln(Z[T/2])-ln(Z[A,2,T])-ln(Z[B,2,T])
// =-2\int_0^{\beta} E - \int_0^{2 \beta} E 
//   + \int_0^{\beta} E_replica_A + \int_0^{\beta} E_replica_B
//here S_2 (A) = S_2 (B) as the cut is symmetric and there is no disorder


#include <iostream>
#include <fstream>
#include <ctime>
#include <boost/multi_array.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <gsl/gsl_integration.h>
#include <boost/lexical_cast.hpp>
#include <interp2d.hpp> // For interpolation
#include <tabdatrw.hpp> // For tabdatr and tabdatw
#include <vector>

// gen is a variable name
// Its data-type is boost::random::mt19937
boost::random::mt19937 gen;
using namespace std;
using boost::lexical_cast;
using boost::bad_lexical_cast;

double J=1.0;
double D = 1.965;;
unsigned int axis1 = 0;
unsigned int axis2 = axis1;
// above assigns length along each dimension of the 2d configuration

//Function templates
double f (double , void * params);
double g (double , void * params);
double h (double , void * params);

int main(int argc, char const * argv[])
{

	if (argc != 5)
	{
		cout << "Expecting four inputs: beta_min, beta_max, del_beta, axis1."
		     << endl << "Got " << argc - 1 << endl;
		return 1;
	}	

	double beta_min(0), beta_max(0), del_beta(0);

	try
	{
		beta_min = lexical_cast<double>(argv[1]);
		beta_max = lexical_cast<double>(argv[2]);
		del_beta = lexical_cast<double>(argv[3]);
	}
	catch (const bad_lexical_cast & x)
	{
		cout << "Cannot convert input to double" << endl;
		return 2;
	}
	try
	{
		axis1 = lexical_cast<unsigned int>(argv[4]);
	}
	catch (const bad_lexical_cast & x)
	{
		cout << "Cannot convert input for axis1 to unsigned int" << endl;
		return 2;
	}
	
	axis2 = axis1;

	string axis_str = lexical_cast<string>(axis1);
	ofstream fout(string("I2" + axis_str + ".dat").c_str());// Opens a file for output
	
	double mut_info(0); //mutual information I_2

	vvdouble vmA = tabdatr(string("Em" + axis_str + ".dat").c_str(), 2);
//modified energy data for replica A
	interp_data idmA(vmA,1);

//	vvdouble vmB = tabdatr("EmB.dat", 2);//modified energy data for B
//	interp_data idmB(vmB,1);

	vvdouble vn = tabdatr(string("E" + axis_str + ".dat").c_str(), 2);
//normal energy data
	interp_data idn(vn,1);
//	gsl_integration_workspace * w
//          = gsl_integration_workspace_alloc (1000);
	gsl_integration_cquad_workspace * w
	    = gsl_integration_cquad_workspace_alloc (100);
	size_t nevals = 1e3;

	for (double beta = beta_min; beta < beta_max + del_beta; beta += del_beta)
	{
		mut_info = 0 ;
		double int_E_T(0), int_E_Tby2(0),int_EA_T(0),abs_error(0);
		gsl_function F;
		F.function = &f;
		F.params = &idn;
//Function: int gsl_integration_qags (const gsl_function * f,double a,double b, double epsabs, double epsrel,size_t limit,gsl_integration_workspace * workspace,double * result, double *abserr)
//gsl_integration_cquad (const gsl_function * f, double a, double b, double epsabs, double epsrel, gsl_integration_cquad_workspace * workspace, double * result, double * abserr, size_t * nevals)
		gsl_integration_cquad (&F, 0,     beta, 1e-6, 1e-4,
		                       w, &int_E_T, &abs_error, &nevals);
		gsl_integration_cquad (&F, 0, 2.0*beta, 1e-6, 1e-4,
		                       w, &int_E_Tby2, &abs_error, &nevals);
		F.function = &g;
		F.params = &idmA;
		gsl_integration_cquad (&F, 0, beta, 1e-6, 1e-4, w, &int_EA_T,
		                       &abs_error, &nevals);

//		F.function = &h;
//		F.params = &idmB;
//		gsl_integration_cquad (&F, 0, beta, 1e-6, 1e-4, w, &int_EB_T,
//		                       &abs_error, &nevals);
// I2 =-2\int_0^{\beta} E - \int_0^{2 \beta} E 
//   + \int_0^{\beta} E_replica_A + \int_0^{\beta} E_replica_B

                mut_info = 2.0*int_EA_T-2.0*int_E_T - int_E_Tby2;
		fout << beta << '\t' << mut_info /axis2 << endl;
	}

	fout.close();
	return 0;
}

//performing numerical integration using gsl
double f (double beta, void * params)
{
	interp_data p = *(interp_data *) params;
	return p.interp_akima(beta);
}

double g (double beta, void * params)
{
	interp_data p = *(interp_data *) params;
	return p.interp_akima(beta);
}

double h (double beta, void * params)
{
	interp_data p = *(interp_data *) params;
	return p.interp_akima(beta);
}




