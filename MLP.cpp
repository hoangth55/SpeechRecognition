#include<assert.h>
#include<iostream>
#include<stdio.h>
#include<math.h>

using namespace std;

class CBackProp{

//	output of each neuron
	double **out;

//	delta error value for each neuron
	double **delta;

//	vector of weights for each neuron
	double ***weight;

//	no of layers in net
//	including input layer
	int numl;

//	vector of numl elements for size 
//	of each layer
	int *lsize;

//	learning rate
	double beta;

//	momentum parameter
	double alpha;

//	storage for weight-change made
//	in previous epoch
	double ***prevDwt;

//	squashing function
	double sigmoid(double in);
	public:

	~CBackProp();

//	initializes and allocates memory
	CBackProp(int nl,int *sz,double b,double a);

//	backpropogates error for one set of input
	void bpgt(double *in,double *tgt);

//	feed forwards activations for one set of inputs
	void ffwd(double *in);

//	returns mean square error of the net
	double mse(double *tgt) const;	
	
//	returns i'th output of the net
	double Out(int i) const;
};


//	initializes and allocates memory on heap
CBackProp::CBackProp(int nl,int *sz,double b,double a):beta(b),alpha(a)
{
	int i;
	//	set no of layers and their sizes
	numl=nl;
	lsize=new int[numl];

	for(int i=0;i<numl;i++){
		lsize[i]=sz[i];
	}

	//	allocate memory for output of each neuron
	out = new double*[numl];

	for(i=0;i<numl;i++){
		out[i]=new double[lsize[i]];
	}

	//	allocate memory for delta
	delta = new double*[numl];

	for(i=1;i<numl;i++){
		delta[i]=new double[lsize[i]];
	}

	//	allocate memory for weights
	weight = new double**[numl];

	for(i=1;i<numl;i++){
		weight[i]=new double*[lsize[i]];
	}
	for(i=1;i<numl;i++){
		for(int j=0;j<lsize[i];j++){
			weight[i][j]=new double[lsize[i-1]+1];
		}
	}

	//	allocate memory for previous weights
	prevDwt = new double**[numl];

	for(i=1;i<numl;i++){
		prevDwt[i]=new double*[lsize[i]];

	}
	for(i=1;i<numl;i++){
		for(int j=0;j<lsize[i];j++){
			prevDwt[i][j]=new double[lsize[i-1]+1];
		}
	}

	//	seed and assign random weights
	srand((unsigned)(time(NULL)));
	for(i=1;i<numl;i++)
		for(int j=0;j<lsize[i];j++)
			for(int k=0;k<lsize[i-1]+1;k++)
				weight[i][j][k]=(double)(rand())/(RAND_MAX/2) - 1;//32767

	//	initialize previous weights to 0 for first iteration
	for(i=1;i<numl;i++)
		for(int j=0;j<lsize[i];j++)
			for(int k=0;k<lsize[i-1]+1;k++)
				prevDwt[i][j][k]=(double)0.0;

// Note that the following variables are unused,
//
// delta[0]
// weight[0]
// prevDwt[0]

//  I did this intentionaly to maintains consistancy in numbering the layers.
//  Since for a net having n layers, input layer is refered to as 0th layer,
//  first hidden layer as 1st layer and the nth layer as output layer. And 
//  first (0th) layer just stores the inputs hence there is no delta or weigth
//  values corresponding to it.
}



CBackProp::~CBackProp()
{
	int i;
	//	free out
	for(int i=0;i<numl;i++)
		delete[] out[i];
	delete[] out;

	//	free delta
	for(i=1;i<numl;i++)
		delete[] delta[i];
	delete[] delta;

	//	free weight
	for(i=1;i<numl;i++)
		for(int j=0;j<lsize[i];j++)
			delete[] weight[i][j];
	for(i=1;i<numl;i++)
		delete[] weight[i];
	delete[] weight;

	//	free prevDwt
	for(i=1;i<numl;i++)
		for(int j=0;j<lsize[i];j++)
			delete[] prevDwt[i][j];
	for(i=1;i<numl;i++)
		delete[] prevDwt[i];
	delete[] prevDwt;

	//	free layer info
	delete[] lsize;
}

//	sigmoid function
double CBackProp::sigmoid(double in)
{
		return (double)(1/(1+exp(-in)));
}

//	mean square error
double CBackProp::mse(double *tgt) const
{
	double mse=0;
	for(int i=0;i<lsize[numl-1];i++){
		mse+=(tgt[i]-out[numl-1][i])*(tgt[i]-out[numl-1][i]);
	}
	return mse/2;
}


//	returns i'th output of the net
double CBackProp::Out(int i) const
{
	return out[numl-1][i];
}

// feed forward one set of input
void CBackProp::ffwd(double *in)
{
	double sum;
	int i;
	//	assign content to input layer
	for(int i=0;i<lsize[0];i++)
		out[0][i]=in[i];  // output_from_neuron(i,j) Jth neuron in Ith Layer

	//	assign output(activation) value 
	//	to each neuron usng sigmoid func
	for(i=1;i<numl;i++){				// For each layer
		for(int j=0;j<lsize[i];j++){		// For each neuron in current layer
			sum=0.0;
			for(int k=0;k<lsize[i-1];k++){		// For input from each neuron in preceeding layer
				sum+= out[i-1][k]*weight[i][j][k];	// Apply weight to inputs and add to sum
			}
			sum+=weight[i][j][lsize[i-1]];		// Apply bias
			out[i][j]=sigmoid(sum);				// Apply sigmoid function
		}
	}
}


//	backpropogate errors from output
//	layer uptill the first hidden layer
void CBackProp::bpgt(double *in,double *tgt)
{
	double sum;
	int i;
	//	update output values for each neuron
	ffwd(in);

	//	find delta for output layer
	for(int i=0;i<lsize[numl-1];i++){
		delta[numl-1][i]=out[numl-1][i]*
		(1-out[numl-1][i])*(tgt[i]-out[numl-1][i]);
	}

	//	find delta for hidden layers	
	for(i=numl-2;i>0;i--){
		for(int j=0;j<lsize[i];j++){
			sum=0.0;
			for(int k=0;k<lsize[i+1];k++){
				sum+=delta[i+1][k]*weight[i+1][k][j];
			}
			delta[i][j]=out[i][j]*(1-out[i][j])*sum;
		}
	}

	//	apply momentum ( does nothing if alpha=0 )
	for(i=1;i<numl;i++){
		for(int j=0;j<lsize[i];j++){
			for(int k=0;k<lsize[i-1];k++){
				weight[i][j][k]+=alpha*prevDwt[i][j][k];
			}
			weight[i][j][lsize[i-1]]+=alpha*prevDwt[i][j][lsize[i-1]];
		}
	}

	//	adjust weights usng steepest descent	
	for(i=1;i<numl;i++){
		for(int j=0;j<lsize[i];j++){
			for(int k=0;k<lsize[i-1];k++){
				prevDwt[i][j][k]=beta*delta[i][j]*out[i-1][k];
				weight[i][j][k]+=prevDwt[i][j][k];
			}
			prevDwt[i][j][lsize[i-1]]=beta*delta[i][j];
			weight[i][j][lsize[i-1]]+=prevDwt[i][j][lsize[i-1]];
		}
	}
}

int main(int argc, char* argv[])
{

	int i;
	// prepare XOR traing data
	double data[][4]={
				0,	0,	0,	0,
				0,	0,	1,	1,
				0,	1,	0,	1,
				0,	1,	1,	0,
				1,	0,	0,	1,
				1,	0,	1,	0,
				1,	1,	0,	0,
				1,	1,	1,	1 };

	// prepare test data	
	double testData[][3]={
								0,      0,      0,
                                0,      0,      1,
                                0,      1,      0,
                                0,      1,      1,
                                1,      0,      0,
                                1,      0,      1,
                                1,      1,      0,
                                1,      1,      1};

	
	// defining a net with 4 layers having 3,3,2, and 1 neuron respectively,
	// the first layer is input layer i.e. simply holder for the input parameters
	// and has to be the same size as the no of input parameters, in out example 3
	int numLayers = 4, lSz[4] = {3,3,2,1};

	
	// Learing rate - beta
	// momentum - alpha
	// Threshhold - thresh (value of target mse, training stops once it is achieved)
	double beta = 0.3, alpha = 0.1, Thresh =  0.00001;

	
	// maximum no of iterations during training
	long num_iter = 2000000;

	
	// Creating the net
	CBackProp *bp = new CBackProp(numLayers, lSz, beta, alpha);
	
	cout<< endl <<  "Now training the network...." << endl;	
	for (long i=0; i<num_iter ; i++)
	{
		
		bp->bpgt(data[i%8], &data[i%8][3]);

		if( bp->mse(&data[i%8][3]) < Thresh) {
			cout << endl << "Network Trained. Threshold value achieved in " << i << " iterations." << endl;
			cout << "MSE:  " << bp->mse(&data[i%8][3]) 
				 <<  endl <<  endl;
			break;
		}
		if ( i%(num_iter/10) == 0 )
			cout<<  endl <<  "MSE:  " << bp->mse(&data[i%8][3]) 
				<< "... Training..." << endl;

	}
	
	if ( i == num_iter )
		cout << endl << i << " iterations completed..." 
		<< "MSE: " << bp->mse(&data[(i-1)%8][3]) << endl;  	

	cout<< "Now using the trained network to make predctions on test data...." << endl << endl;	
	for ( i = 0 ; i < 8 ; i++ )
	{
		bp->ffwd(testData[i]);
		cout << testData[i][0]<< "  " << testData[i][1]<< "  "  << testData[i][2]<< "  " << bp->Out(0) << endl;
	}
	system("pause");
	return 0;
}



