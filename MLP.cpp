#include<assert.h>
#include<iostream>
#include<stdio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include<fstream>

using namespace std;

int numOfElement , numOfTestCase;
//#define trainingSet = "train.inp";
//#define testSet = "test.inp";
double **desiredOut;

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
// read traning set from file
	void trainedSet(double** , const char*);

// read test set from file
    void testedSet(double** , const char*);

    int maxResult();
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

/*double CBackProp::purelin(double s){
    return s;
}*/

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

int CBackProp::maxResult(){
    double likely = Out(0);
    int index = 0;
    for (int i = 1 ; i < 10 ; i++)
        if (likely < Out(i)){
            likely = Out(i);
            index = i;
        }
    return index;
}

void trainedSet(double **&data , const char* fileName){
    int i , j,x;
    ifstream fin ;
    fin.open(fileName);
    if (!fin.is_open()){
        cout << "Can't open file to train " << endl;
        exit(1);
    }
    fin >> numOfElement;
    desiredOut = new double*[numOfElement];
    data = new double*[numOfElement];
    for (i = 0 ; i < numOfElement ; i ++){
        data[i] = new double[101];
        desiredOut[i] = new double[10];
    }
    for (i = 0 ; i < numOfElement ; i ++){
        for (j = 0 ; j < 100 ; j ++)
            fin >> data[i][j];
        fin >> x;
        desiredOut[i][x] = 1;
    }
}

void testedSet(double **&testSet , const char* fileName){
    int i , j;
    ifstream fin;
    fin.open(fileName);
    if (!fin.is_open()){
        cout << "Can't open file to test " << endl;
        exit(1);
    }
    fin >> numOfTestCase;
    testSet  = new double*[numOfTestCase];
    for (i = 0 ; i < numOfTestCase ; i++)
        testSet[i] = new double[100];
    for (i = 0 ; i < numOfTestCase ; i ++){
        for (j = 0 ; j < 100 ; j ++){
            fin >> testSet[i][j];
        }
    }
}

int main(int argc, char* argv[])
{

	long i;
	double **trainData;
	double **testData;
	int result;
    ofstream fout;
    fout.open("result.out");
    if (!fout.is_open()){
        cout << "Can't open output file " << endl;
        exit(1);
    }
	trainedSet(trainData,"trainingSet.inp");
	testedSet(testData,"testingSet.inp");

	// defining a net with 4 layers having 3,3,2, and 1 neuron respectively,
	// the first layer is input layer i.e. simply holder for the input parameters
	// and has to be the same size as the no of input parameters, in out example 3
	int numLayers = 4, lSz[4] = {101,4,3,10};


	// Learing rate - beta
	// momentum - alpha
	// Threshhold - thresh (value of target mse, training stops once it is achieved)
	double beta = 0.3, alpha = 0.1, Thresh =  0.00001;


	// maximum no of iterations during training
	long num_iter = 200000;


	// Creating the net
	CBackProp *bp = new CBackProp(numLayers, lSz, beta, alpha);

	cout<< endl <<  "Now training the network...." << endl;
	for (i=0; i<num_iter ; i++)
	{

		bp->bpgt(trainData[i%numOfElement], desiredOut[i%numOfElement]);

		if( bp->mse(desiredOut[i%numOfElement]) < Thresh) {
			cout << endl << "Network Trained. Threshold value achieved in " << i << " iterations." << endl;
			cout << "MSE:  " << bp->mse(desiredOut[i%numOfElement])
				 <<  endl <<  endl;
			break;
		}
		if ( i%(num_iter/10) == 0 )
			cout<<  endl <<  "MSE:  " << bp->mse(desiredOut[i%numOfElement])
				<< "... Training..." << endl;
	}

	if ( i == num_iter )
		cout << endl << i << " iterations completed..."<< endl;

	cout<< "Now using the trained network to make predctions on test data...." << endl << endl;
	for ( i = 0 ; i < numOfTestCase ; i++ )
	{
		bp->ffwd(testData[i]);
		result = bp->maxResult();
		fout << result << endl;
	}
	system("pause");
	return 0;
}






