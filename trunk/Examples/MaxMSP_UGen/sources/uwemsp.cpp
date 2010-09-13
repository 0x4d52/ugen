#include "uwemsp.h"
#include "../../../UGen/UGen.h"

typedef struct _uwemsp
{
	
	// you must have a t_pxobject first instead of a t_object for MSP
	t_pxobject u_pxob;
	
	// we will store some into in this when audio is turned on
	// so we know how to behave when processing audio data
	// this is a little less error prone than changing the dsp function
	t_float** u_buffers;
	int u_buffersize;
	
	// then you can have an other fields you need in your object 
	// ...
	
	float u_param;
	
	UGen u_input;
	UGen u_synth;
	
} t_uwemsp;

// convenient way to configure inlets/outlets, only one in, one out here
typedef enum {
	UWE_SIG_INPUT0,
	UWE_SIG_INPUT1, // etc...
	
	UWE_SIG_INPUTS_COUNT	// total count of signal inlets
} e_uwe_sig_inlets;

typedef enum {
	UWE_SIG_OUTPUT0 = UWE_SIG_INPUTS_COUNT,  // start enum after signal input indices
	UWE_SIG_OUTPUT1, // etc...
	
	UWE_SIG_IO_COUNT		// total count of signal inlets and outlets
} e_uwe_sig_outlets;

// calculate total count of signal outlets
#define UWE_SIG_OUTPUTS_COUNT (UWE_SIG_IO_COUNT - UWE_SIG_INPUTS_COUNT)

// convenient define to make a note of how many arguments to pass
// to our perform function
#define UWE_ARGUMENTS_PASSED_TO_PERFORM 1
#define UWE_PERFORM_RETURN(w) (w+UWE_ARGUMENTS_PASSED_TO_PERFORM+1)


// commonly needed functions for creating/deleteing objects and giving assistance info
void *uwemsp_new(t_symbol *s, long argc, t_atom *argv);
void uwemsp_free(t_uwemsp *x);
void uwemsp_assist(t_uwemsp *x, void *unused, long assist_type, long assist_index, char *assist_string);

// two additional functions are needed to for MSP 
// 1. a dsp function which is called as audio is turned on
// 2. a perform function which processes each block of audio
void uwemsp_dsp(t_uwemsp *x, t_signal **sp, short *count);
t_int *uwemsp_perform(t_int *w);

// Additional functions can be declared here, these also need to be defined, AND registered with the
// class in the main() function using the class_addmethod() function. This is so that Max messages
// can cause our object's funtions to be called.
void uwemsp_param(t_uwemsp *x, double value);

// We need a global variable for our Max class, this an OK use of global variables!
t_class *uwemsp_class;


int main(void)
{	
	// temporary pointer to our class while we build its features and register it with Max
	t_class *c;
	
	// NB. MAXOBJECT_NAME is #define'd to be the name of your project (Xcode) or solution (Visual Studio)
	// You MUST make sure your project/solution name contains only alphanumeric characters and underscores
	// and doesn't start with a number. e.g., my_new_object, myVeryFirstMaxObject, etc.
	
	// MSPOBJECT_NAME is MAXOBJECT_NAME with a "~" character added
	
	c = class_new(MSPOBJECT_NAME, (method)uwemsp_new, (method)uwemsp_free, (long)sizeof(t_uwemsp), 0L, A_GIMME, 0);
	
	// add a method for Max to obtain assistance from our object
    class_addmethod(c,			(method)uwemsp_assist,				"assist",		A_CANT,			0);
	
	// add a method for Max to setup our dsp
    class_addmethod(c,			(method)uwemsp_dsp,					"dsp",			A_CANT,			0);
	
	// add other methods for Max to communicate with our object
	//				class		function callback					Max message		argument		terminated
	//				pointer		casted to a "method"				to respond to	type(s)			with zero
	class_addmethod(c,			(method)uwemsp_param,				"gain",			A_FLOAT,			0); 
		
	// register our object class as a normal "box" type Max object
	class_register(gensym("box"), c);
	
	// initialise the dsp aspects of our object
	class_dspinit(c);
	
	// keep a reference to this class in a global variable
	uwemsp_class = c;				
	
	// optional announcement that the object has been loaded (NOT instantiated!)
	post("I am the %s object", MSPOBJECT_NAME);
		
	UGen::initialise();
	
	// no error occured, so return zero
	return 0;
}

void uwemsp_assist(t_uwemsp *x, void *unused, long assist_type, long assist_index, char *assist_string)
{
	if (assist_type == ASSIST_INLET) 
	{
		// you could use a switch/case here to deal with each inlet
		sprintf(assist_string, "I am inlet %ld", assist_index);
	} 
	else // outlet
	{	
		// you could use a switch/case here to deal with each outlet
		sprintf(assist_string, "I am outlet %ld", assist_index); 			
	}
}

void uwemsp_free(t_uwemsp *x)
{
	dsp_free((t_pxobject *)x); // needs to be called for MSP objects on free
	
	// delete our array of signal pointers
	sysmem_freeptr(x->u_buffers);
}


void *uwemsp_new(t_symbol *s, long argc, t_atom *argv)
{
	// Max calls this function when it needs an instance of your object
	
	t_uwemsp *x = 0;
    int count;

	if (x = (t_uwemsp *)object_alloc(uwemsp_class)) 
	{
		// add signal inlets (these can be used for other messages
		dsp_setup((t_pxobject *)x, UWE_SIG_INPUTS_COUNT);
		x->u_pxob.z_misc |= Z_NO_INPLACE;
		
		// regular outlets would need adding here, before signal outets
		//...
		
		// add signal outlets
		for(count = 0; count < UWE_SIG_OUTPUTS_COUNT; count++)
		{
			outlet_new(x, "signal");
		}
		
		// allocate an array to store our signal pointers in
		x->u_buffers = (t_float**)sysmem_newptrclear(sizeof(t_float**) * UWE_SIG_IO_COUNT);
		
		// initialise any other data
		x->u_param = 1.f;
		
		
	}
	
	// return a pointer to our object back to Max
	return (x);
}

void uwemsp_dsp(t_uwemsp *x, t_signal **sp, short *count)
{
	// store info in our object for when we process data
	int index;
	for(index = 0; index < UWE_SIG_IO_COUNT; index++)
	{
		x->u_buffers[index] = sp[index]->s_vec;
	}
	x->u_buffersize = sp[0]->s_n;
	
	// register our perform function callback with our struct 
	// as a parameter to pass to it
	dsp_add(uwemsp_perform, UWE_ARGUMENTS_PASSED_TO_PERFORM, x);
	UGen::prepareToPlay(sp[0]->s_sr, x->u_buffersize, x->u_buffersize);
		
	x->u_input = AudioIn::AR(2);
	x->u_synth = x->u_input * Lag::AR(&x->u_param);
}

t_int *uwemsp_perform(t_int *w)
{
	// get our struct back out of the w[] array
	t_uwemsp *x = (t_uwemsp *)(w[1]);
	
	// get the signal info from our struct
	int numsamples  = x->u_buffersize;
	
	// obtain a new UGen blockID before commencing the processing of each block
	int blockID = UGen::getNextBlockID(numsamples);
	
	// tell our UGen graph where to read data from (if applicable)
	x->u_input.setInputs((const float**)x->u_buffers, numsamples, UWE_SIG_INPUTS_COUNT);
	
	// tell our UGen graph where to write its data
	x->u_synth.setOutputs(x->u_buffers + UWE_SIG_OUTPUT0, numsamples, UWE_SIG_OUTPUTS_COUNT);
	
	// prepare, and process the block
	x->u_synth.prepareAndProcessBlock(numsamples, blockID, -1);
	
	// always return w plus one more than the number of 
	// args passed to this function in w[]
	return UWE_PERFORM_RETURN(w);
}

void uwemsp_param(t_uwemsp *x, double value)
{
	// store the number from Max in our object struct
	x->u_param = value;
}


