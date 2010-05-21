#ifndef _MAINCOMPONENT_H_
#define _MAINCOMPONENT_H_

#include <juce/juce.h>

class UGenComponent : public Component
	{	
	public:
		
		enum IOType { Input, Output, Other };
		enum IOPosition { Left, Top, Right, Bottom };
		
		UGenComponent (String ugenClass = T("UGen"), 
					   StringArray const& inputs = StringArray(), 
					   StringArray const& outputs = StringArray(), 
					   String ugenName = String::empty,
					   IOPosition inputsPos = Top,
					   IOPosition outputsPos = Bottom,
					   StringArray const& other = StringArray(),
					   IOPosition otherPos = Right)
		:	ugenClass_(ugenClass),
			inputs_(inputs),
			outputs_(outputs),
			ugenName_(ugenName),
			largeFontSize(18),
			smallFontSize(9),
			cornerSize(8),
			inputsPos_(inputsPos),
			outputsPos_(outputsPos),
			other_(other),
			otherPos_(otherPos)
		{
		}
		
		~UGenComponent ()
		{
			deleteAllChildren();
		}
		
		void resized ()
		{
			repaint();
		}
		
		void moved ()
		{
			getParentComponent()->repaint();
		}
		
		void paint (Graphics& g)
		{
			g.setColour(Colours::white);
			g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), cornerSize);
			g.setColour(Colours::black);
			g.drawRoundedRectangle(0, 0, getWidth(), getHeight(), cornerSize, 2);
			g.setFont(largeFontSize);
			g.drawText(ugenClass_, 0, 0, getWidth(), getHeight()-smallFontSize, Justification::centred, false);
			
			if(ugenName_ != String::empty)
			{
				g.setFont(smallFontSize);
				g.drawText(ugenName_, 0, smallFontSize, getWidth(), getHeight(), Justification::centred, false);
			}
			
			const int numInputs = inputs_.size();
			if(numInputs > 0)
			{
				if(inputsPos_ == Top || inputsPos_ == Bottom)
				{
					const int inputWidth = (getWidth()-cornerSize-cornerSize)/numInputs;
					for(int i = 0; i < numInputs; i++)
					{
						g.setFont(smallFontSize);
						const int y = inputsPos_ == Top ? 0 : getHeight()-smallFontSize;
						g.drawText(inputs_[i], 
								   inputWidth * i + cornerSize + 1, y, inputWidth - 1, smallFontSize, 
								   Justification::centred, false);
					}
				}
				else // inputs are left or right
				{
					const int inputHeight = (getHeight()-cornerSize-cornerSize)/numInputs;
					for(int i = 0; i < numInputs; i++)
					{
						g.setFont(smallFontSize);
						Justification just = inputsPos_ == Left ? Justification::centredLeft :  Justification::centredRight;
						g.drawText(inputs_[i], 
								   0, inputHeight * i + cornerSize, getWidth(), inputHeight,
								   just, false);
					}
				}
			}
			
			const int numOutputs = outputs_.size();
			if(numOutputs > 0)
			{
				if(outputsPos_ == Top || outputsPos_ == Bottom)
				{
					const int outputWidth = (getWidth()-cornerSize-cornerSize)/numOutputs;
					for(int i = 0; i < numOutputs; i++)
					{
						g.setFont(smallFontSize);
						const int y = outputsPos_ == Top ? 0 : getHeight()-smallFontSize;
						g.drawText(outputs_[i], 
								   outputWidth * i + cornerSize + 1, y, outputWidth - 1, smallFontSize, 
								   Justification::centred, false);
					}
				}
				else // outputs are left or right
				{
					const int outputHeight = (getHeight()-cornerSize-cornerSize)/numOutputs;
					for(int i = 0; i < numOutputs; i++)
					{
						g.setFont(smallFontSize);
						Justification just = outputsPos_ == Left ? Justification::centredLeft :  Justification::centredRight;
						g.drawText(outputs_[i], 
								   0, outputHeight * i + cornerSize, getWidth(), outputHeight,
								   just, false);
					}
				}
			}
		}
		
		void mouseDown (const MouseEvent& e)
        {
            dragger.startDraggingComponent (this, 0);
        }
		
        void mouseDrag (const MouseEvent& e)
        {
            dragger.dragComponent (this, e);
        }
		
		bool getInputPosition(String const& inputName, int& x, int& y, IOPosition& pos)
		{
			pos = inputsPos_;
			
			if(inputs_.contains(inputName, false))
				return getInputPosition(inputs_.indexOf(inputName), x, y, pos);
			else
				return false;
		}
		
		bool getInputPosition(const int inputIndex, int& x, int& y, IOPosition& pos)
		{
			const int numInputs = inputs_.size();
			
			pos = inputsPos_;
			
			if(inputIndex < 0 || inputIndex >= inputs_.size())
				return false;
			else
			{
				if(inputsPos_ == Top || inputsPos_ == Bottom)
				{
					const int inputWidth = (getWidth()-cornerSize-cornerSize)/numInputs;
					x = getX() + inputWidth * inputIndex + cornerSize + inputWidth/2;
					y = inputsPos_ == Top ? getY() : getBottom();
					return true;
				}
				else // inputs are left or right
				{
					const int inputHeight = (getHeight()-cornerSize-cornerSize)/numInputs;
					x = inputsPos_ == Left ? getX() : getRight();
					y = getY() + inputHeight * inputIndex + cornerSize + inputHeight/2;
					return true;
				}
			}
		}
		
		bool getOutputPosition(String const& outputName, int& x, int& y, IOPosition& pos)
		{
			pos = outputsPos_;
			
			if(outputs_.contains(outputName, false))
				return getOutputPosition(outputs_.indexOf(outputName), x, y, pos);
			else
				return false;
		}
		
		bool getOutputPosition(const int outputIndex, int& x, int& y, IOPosition& pos)
		{
			const int numOutputs = outputs_.size();
			
			pos = outputsPos_;
			
			if(outputIndex < 0 || outputIndex >= outputs_.size())
				return false;
			else
			{
				if(outputsPos_ == Top || outputsPos_ == Bottom)
				{
					const int outputWidth = (getWidth()-cornerSize-cornerSize)/numOutputs;
					x = getX() + outputWidth * outputIndex + cornerSize + outputWidth/2;
					y = outputsPos_ == Top ? getY() : getBottom();
					return true;
				}
				else // outputs are left or right
				{
					const int outputHeight = (getHeight()-cornerSize-cornerSize)/numOutputs;
					x = outputsPos_ == Left ? getX() : getRight();
					y = getY() + outputHeight * outputIndex + cornerSize + outputHeight/2;
					return true;
				}
			}
		}
		
		const StringArray& getInputs()	{ return inputs_; };
		const StringArray& getOutputs() { return outputs_; };
		const StringArray& getOthers()	{ return other_; };
		const String& getName()			{ return ugenName_; }
		const String& getClass()		{ return ugenClass_; }
		
	private:
		ComponentDragger dragger;
		String ugenClass_;
		StringArray inputs_;
		StringArray outputs_;
		String ugenName_;
		int largeFontSize;
		int smallFontSize;
		int cornerSize;
		
		IOPosition inputsPos_;
		IOPosition outputsPos_;
		
		StringArray other_;
		IOPosition otherPos_;
	};

class UGenWire : public Component
{
public:	
	UGenWire(UGenComponent* src, const int outputIndex, 
			 UGenComponent* dst, const int inputIndex) : src_(0), dst_(0)
	{
		init(src, UGenComponent::Output, outputIndex,
			 dst, UGenComponent::Input, inputIndex);
	}
	
	UGenWire(UGenComponent* src, String const& outputName, 
			 UGenComponent* dst, String const& inputName) : src_(0), dst_(0)
	{
		const int outputIndex = src->getOutputs().indexOf(outputName);
		const int inputIndex = dst->getInputs().indexOf(inputName);
		init(src, UGenComponent::Output, outputIndex,
			 dst, UGenComponent::Input, inputIndex);
	}
	
	UGenWire(UGenComponent* src, UGenComponent::IOType srcType, const int outputIndex, 
			 UGenComponent* dst, UGenComponent::IOType dstType, const int inputIndex) : src_(0), dst_(0)
	{
		init(src, srcType, outputIndex,
			 dst, dstType, inputIndex);
	}
	
	UGenWire(UGenComponent* src, UGenComponent::IOType srcType, String const& outputName, 
			 UGenComponent* dst, UGenComponent::IOType dstType, String const& inputName) : src_(0), dst_(0)
	{
		const int outputIndex = src->getOutputs().indexOf(outputName);
		const int inputIndex = dst->getInputs().indexOf(inputName);
		init(src, srcType, outputIndex,
			 dst, dstType, inputIndex);
	}
	
	~UGenWire()
	{
		
	}
	
	void resized ()
	{
		//repaint();
	}
	
	void paint (Graphics& g)
	{
		g.setColour(Colours::red);
		
		int startX, startY, endX, endY, finalX, finalY;
		UGenComponent::IOPosition outPos, inPos;
		src_->getOutputPosition(outputIndex_, startX, startY, outPos);
		dst_->getInputPosition(inputIndex_, endX, endY, inPos);
		
		const int curveOffset = jmax(24, (endY-startY)/2);
		const int arrowSize = 8;
		Path path;
		path.startNewSubPath(startX, startY);
		
		int control1X, control2X, control1Y, control2Y;
		
		switch(outPos)
		{
			case UGenComponent::Top: {
				control1X = startX;
				control1Y = startY-curveOffset;
			} break;
			case UGenComponent::Bottom: {
				control1X = startX;
				control1Y = startY+curveOffset;
			} break;
			case UGenComponent::Left: {
				control1X = startX-curveOffset;
				control1Y = startY;
			} break;
			case UGenComponent::Right: {
				control1X = startX+curveOffset;
				control1Y = startY;
			} break;
		}
		
		switch(inPos)
		{
			case UGenComponent::Top: {
				control2X = endX;
				control2Y = endY-curveOffset-arrowSize;
				finalX = endX;
				finalY = endY-arrowSize;
			} break;
			case UGenComponent::Bottom: {
				control2X = endX;
				control2Y = endY+curveOffset+arrowSize;
				finalX = endX;
				finalY = endY+arrowSize;
			} break;
			case UGenComponent::Left: {
				control2X = endX-curveOffset-arrowSize;
				control2Y = endY;
				finalX = endX-arrowSize;
				finalY = endY;
			} break;
			case UGenComponent::Right: {
				control2X = endX+curveOffset+arrowSize;
				control2Y = endY;
				finalX = endX+arrowSize;
				finalY = endY;
			} break;
		}
		
		path.cubicTo(control1X, control1Y, 
					 control2X, control2Y, 
					 finalX, finalY);
		path.addArrow(finalX, finalY,
					  endX, endY,
					  2, arrowSize, arrowSize);
		
		g.strokePath(path, PathStrokeType(2.f));
	}
	
private:
	void init(UGenComponent* src, UGenComponent::IOType srcType, const int outputIndex, 
			  UGenComponent* dst, UGenComponent::IOType dstType, const int inputIndex)
	{
		setInterceptsMouseClicks(false, false);
		
		if(outputIndex >= 0 && inputIndex >= 0)
		{
			src_ = src;
			dst_ = dst;
			outputIndex_ = outputIndex;
			inputIndex_ = inputIndex;
			srcType_ = srcType;
			dstType_ = dstType;
		}
	}
	
	UGenComponent* src_;
	UGenComponent* dst_;
	int outputIndex_, inputIndex_;
	UGenComponent::IOType srcType_, dstType_;
};

class UGenGraphComponent  : public Component      
{
private:		
	Array<UGenComponent*> ugens;
	Array<UGenWire*> wires;
	
public:
	UGenGraphComponent ()
	{
		setInterceptsMouseClicks(false, true);
	}
	
	~UGenGraphComponent ()
	{
		deleteAllChildren();
	}
	
	void addAndMakeVisible (UGenComponent* const child)
	{
		Component::addAndMakeVisible(child);
		ugens.add(child);
	}
	
	void addAndMakeVisible (UGenWire* const child)
	{
		Component::addAndMakeVisible(child);
		wires.add(child);
	}
	
	void resized ()
	{			
		for(int i = 0; i < wires.size(); i++)
		{
			wires[i]->setBounds(0, 0, getWidth(), getHeight());
		}
		
		repaint();
	}
	
	UGenComponent* getUGenComponent(const int index)
	{
		return ugens[0];
	}
	
	UGenComponent* getUGenComponent(String const& name)
	{
		for(int i = 0; i < ugens.size(); i++)
		{
			if(name == ugens[i]->getName())
				return ugens[i];
		}
		
		return 0;
	}
	
};

class MainComponent : public Component 
{
public:
	MainComponent() 
	{
		addAndMakeVisible(graph = new UGenGraphComponent());
		
		const char* inputs[] = {"freq", "phase", "mul", "add", 0};
		const char* outputs[] = {"out", 0};
		graph->addAndMakeVisible(new UGenComponent(T("SinOsc"), inputs, outputs, T("modulator")));//, UGenComponent::Top, UGenComponent::Right));
		graph->addAndMakeVisible(new UGenComponent(T("SinOsc"), inputs, outputs, T("carrier")));
		graph->addAndMakeVisible(new UGenWire(graph->getUGenComponent(T("modulator")), T("out"), 
											  graph->getUGenComponent(T("carrier")), T("freq")));
		graph->addAndMakeVisible(new UGenWire(graph->getUGenComponent(T("carrier")), T("out"),
											  graph->getUGenComponent(T("modulator")), T("mul")));
		
		
		graph->getUGenComponent(T("modulator"))->setBounds(30, 30, 100, 80);
		graph->getUGenComponent(T("carrier"))->setBounds(130, 180, 100, 80);
	}
	
	~MainComponent()
	{
		deleteAllChildren();
	}
	
	void resized()
	{
		graph->setBounds(0, 0, getWidth(), getHeight());
	}
	
private:
	UGenGraphComponent* graph;
};

#endif //_MAINCOMPONENT_H_ 