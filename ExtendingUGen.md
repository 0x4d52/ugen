# Introduction #

This guide will briefly go through how to develop your own UGen class for custom processing. As well as serving as a tutorial on how to achieve this it aims to put in place some style guidelines to keep code consistent with the rest of the library.

It also aims to help explain some of the macros involved in UGen which help to write classes quickly and eliminate duplicate code.


# Details #

This guide will go through how to develop the Maxima UGen. This is a simple UGen which will scan through a given UGen buffer and output 1's at the maximum points in the buffer, otherwise it will fill the buffer with 0's.

For viewing ease the complete class is shown at the bottom of this guide with code snippets presented throughout as they are discussed.

## Class Declaration ##

Firstly you will need to declare a new class which inherits from UGenInternal. UGenInternal's deal with the actual processing of the unit while the associated UGen class is mainly used to connect UGens in a graph. In this example most of the associated UGen will be created using built in macros to the library.

The UGen naming conventions state that a UGenInternal should have the same name as its corresponding UGen with UGenInternal appended to it. Also do not forget to include the header guards and main UGen header as described in the main style guide.

```
#ifndef _UGEN_Maxima_H_
#define _UGEN_Maxima_H_

#include "../core/ugen_UGen.h"

// A new UGen also needs a UGenInternal which does the actual processing. 
// UGen and its subclasses are primarily for constructing the UGen graphs.
// Simle internals should inherit from UGenInternal.
class MaximaUGenInternal : public UGenInternal
{
```

The first thing to be declared in your class should be the constructor. This takes its parent UGen as an argument and by convention is named _input_. When using your own UGen, you declare and instantiate the parent UGen which will create the required number of UGenInternals automatically for you and pass the processing onto them seamlessly.

Also requied is a _processBlock_ function that will perfrom the DSP of the UGen and a list of inputs as an enum. In this instance there is only one input called input.

```
public:
	// Constructor.
	MaximaUGenInternal(UGen const& input) throw();
	
	// This is called when the internal is needed to process a new block of samples.
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs }; // used mainly by the 'inputs' array for the UGenInternal's UGen inputs
```

After this you can declare your own member variables. In this case they are declared _protected_ in case any subclasses need to access them. The two variables used here are a flag to keep track of the last input increase and a float variable to hold the last input value. After this we can close the class declaration.

```
protected:
	bool didIncreaseLastTime;
	float lastValue;
};
```

The next thing to add in the header file is a handy macro for generating a generic UGen class interface. Again naming conventions state that the name should be the same as that as the internal implimentation.

There are a variety of macros that you can use for this purpose but the simplest is the one shown below which takes the name of the UGen class to generate, a constructor implementation argument list (in this just the input) and the constructor declaration argument list. Also required is the documentation section to put this class into, in this case it is just the `COMMON_UGEN_DOCS`.

```
// A macro for declaring a UGen subclass.
// The parentheses around the 2nd and 3rd arguments are required.
// Items within these parentheses should be comma-separated just as for argument declarations and calls.
UGenSublcassDeclarationNoDefault
(
	Maxima,					// The UGen name, this will inherit from UGen
	(input),				// argument list for the Constructor, AR and KR methods as they would be CALLED
	(UGen const& input),	// argument list for the Constructor, AR and KR methods as they are to be DECLARED
	COMMON_UGEN_DOCS		// Documentation (for Doxygen)
);
```

Lastly is a quick macro that enables the user to call the default constructor of the UGen and instantiate it without the usually necessary parenthesis. Also required is the closing heading guard.

```
#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define Maxima Maxima()
#endif


#endif // _UGEN_Maxima_H_
```

## Class Implementation ##

Once your class has been declared you will need to write the implementation of it. This is usually done in a .cpp file with the same name as the header. First you need to include the UGen standard header which defines various macros for you, implement the UGen namespace macro and include your header file.

```
#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Maxima.h"
```

Now you need to implement your constructor. Remember as your class inherits from UGenInternal you also need to initialise the base class with the number of inputs your UGen requires. Then you can set the internal inputs array to your class's input and initialise any member variables you may have created.

```
MaximaUGenInternal::MaximaUGenInternal(UGen const& input) throw()
:	UGenInternal(NumInputs) // Initialise the UGenIntenal's UGen inputs array using the enum size.
{
	inputs[Input] = input;
	didIncreaseLastTime = false;
	lastValue = 0.f;
}
```

Now you can impliment the real nuts and bolts of your class, the processBlock. This is where all of your class's processing will be performed. Although it is not really the point of this guide to go through how the Maxima class works a brief description is provided below.

It is a good idea to initialise some variables from the UGenInternal parent class's member variables such as the number of samples to process, a pointer to the output buffer and the input buffer.

The while loop then implements the dsp algorithm specific to this UGen. For the maxima ugen this involves searching the input buffer and comparing the current value to the previous one. If the sample in the buffer has decreased from the previous one but the previous one had increased a one is written to the output buffer. For all other cases a 0 is written. This gives an output buffer full of 0's and 1's with the 1's only where a peak in the buffer occurs.

```
void MaximaUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	// Get the number of samples to process this block.
	int numSamplesToProcess = uGenOutput.getBlockSize();
	
	// Get a pointer to the output sample array.
	float* outputSamples = uGenOutput.getSampleData();
	
	// Get a pointer to the input sample array.
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	
	// The dsp loop:
        while(numSamplesToProcess--)
        {
                float input = *inputSamples;    // Pull a sample out of the input array.
                
                bool didIncreaseThisTime;
                
                if(input > lastValue)
                        didIncreaseThisTime = true;
                else
                        didIncreaseThisTime = false;
                
                float output;
                
                if((didIncreaseThisTime == false) && (didIncreaseLastTime == true))
                        output = 1.f;
                else
                        output = 0.f;
                
                lastValue = input;
                didIncreaseLastTime = didIncreaseThisTime;
                
                *outputSamples = output;                // Put a sample into the output array.
                
                // Increment the array pointers for the next loop iteration.
                inputSamples++;         
                outputSamples++;
        }

}
```

Next we have to implement the parent UGen constructor that was automatically generated by our macro in the header file. All this does is instantiate the required number of UGenInternals to carry out the processing required but the current input.

```
Maxima::Maxima(UGen const& input) throw()
{
	initInternal(input.getNumChannels());
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new MaximaUGenInternal(input);
	}
}
```

Finally we use the UGen macro to finish using the UGen namespace.

```
END_UGEN_NAMESPACE
```

As you can see writing your own UGen is quite simple. Just follow the steps here substituting the names and processBlock algorithm and you should be well on your way to helping develop the UGen library.


# Complete Code #

### ugen\_Maxima.h ###
```
#ifndef _UGEN_Maxima_H_
#define _UGEN_Maxima_H_

#include "../core/ugen_UGen.h"

// A new UGen also needs a UGenInternal which does the actual processing. 
// UGen and its subclasses are primarily for constructing the UGen graphs.
// Simle internals should inherit from UGenInternal.
class MaximaUGenInternal : public UGenInternal
{
public:
	// Constructor.
	MaximaUGenInternal(UGen const& input) throw();
	
	// This is called when the internal is needed to process a new block of samples.
	void processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw();
	
	enum Inputs { Input, NumInputs }; // used mainly by the 'inputs' array for the UGenInternal's UGen inputs
	
protected:
	bool didIncreaseLastTime;
	float lastValue;
};

// A mocro for declaring a UGen subclass.
// The parentheses around the 2nd and 3rd arguments are required.
// Items within these parentheses should be comma-separated just as for argument declarations and calls.
UGenSublcassDeclarationNoDefault
(
	Maxima,					// The UGen name, this will inherit from UGen
	(input),				// argument list for the Constructor, AR and KR methods as they would be CALLED
	(UGen const& input),	// argument list for the Constructor, AR and KR methods as they are to be DECLARED
	COMMON_UGEN_DOCS		// Documentation (for Doxygen)
);

#if defined(UGEN_USER_MODE) && defined(UGEN_SCSTYLE)
#define Maxima Maxima()
#endif


#endif // _UGEN_Maxima_H_
```

### ugen\_Maxima.cpp ###
```
#include "../core/ugen_StandardHeader.h"

BEGIN_UGEN_NAMESPACE

#include "ugen_Maxima.h"

MaximaUGenInternal::MaximaUGenInternal(UGen const& input) throw()
:	UGenInternal(NumInputs) // Initialise the UGenIntenal's UGen inputs array using the enum size.
{
	inputs[Input] = input;
	didIncreaseLastTime = false;
	lastValue = 0.f;
}

void MaximaUGenInternal::processBlock(bool& shouldDelete, const unsigned int blockID, const int channel) throw()
{
	// Get the number of samples to process this block.
	int numSamplesToProcess = uGenOutput.getBlockSize();
	
	// Get a pointer to the output sample array.
	float* outputSamples = uGenOutput.getSampleData();
	
	// Get a pointer to the input sample array.
	float* inputSamples = inputs[Input].processBlock(shouldDelete, blockID, channel);
	
        // The dsp loop:
        while(numSamplesToProcess--)
        {
                float input = *inputSamples;    // Pull a sample out of the input array.
                
                bool didIncreaseThisTime;
                
                if(input > lastValue)
                        didIncreaseThisTime = true;
                else
                        didIncreaseThisTime = false;
                
                float output;
                
                if((didIncreaseThisTime == false) && (didIncreaseLastTime == true))
                        output = 1.f;
                else
                        output = 0.f;
                
                lastValue = input;
                didIncreaseLastTime = didIncreaseThisTime;
                
                *outputSamples = output;                // Put a sample into the output array.
                
                // Increment the array pointers for the next loop iteration.
                inputSamples++;         
                outputSamples++;
        }

}

Maxima::Maxima(UGen const& input) throw()
{
	initInternal(input.getNumChannels());
	
	for(int i = 0; i < numInternalUGens; i++)
	{
		internalUGens[i] = new MaximaUGenInternal(input);
	}
}

END_UGEN_NAMESPACE
```