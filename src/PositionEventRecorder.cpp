
#define DEBUG_LEVEL 1
#include <bbcat-render/Event.h>
#include <bbcat-render/SelfRegisteringControlReceiver.h>
#include "PositionEventRecorder.h"

BBC_AUDIOTOOLBOX_START

static const struct {
  PARAMETERDESC filename;
} _parameters = 
{
  {"filename",   "Filename of event file to create"},
};

SELF_REGISTERING_CONTROL_RECEIVER(PositionEventRecorder, "event.positionwriter");

PositionEventRecorder::PositionEventRecorder() : SoundPositionConsumer()
{
}

PositionEventRecorder::PositionEventRecorder(const ParameterSet& parameters) : SoundPositionConsumer(parameters)
{
  SetParameters(parameters);
}

PositionEventRecorder::~PositionEventRecorder()
{
}

/*--------------------------------------------------------------------------------*/
/** Set parameters within object (*only* parameters that can be set more than once)
 */
/*--------------------------------------------------------------------------------*/
void PositionEventRecorder::SetParameters(const ParameterSet& parameters)
{
  SoundPositionConsumer::SetParameters(parameters);

  parameters.Get(_parameters.filename.name, filename);
}

/*--------------------------------------------------------------------------------*/
/** Get a list of parameters for this object
 */
/*--------------------------------------------------------------------------------*/
void PositionEventRecorder::GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  const PARAMETERDESC *pparameters = (const PARAMETERDESC *)&_parameters;
  uint_t i, n = sizeof(_parameters) / sizeof(pparameters[0]);

  SoundPositionConsumer::GetParameterDescriptions(list);

  for (i = 0; i < n; i++) list.push_back(pparameters + i);
}

/*--------------------------------------------------------------------------------*/
/** Overridable update position function 
 *
 * @param channel channel to change the position of
 * @param pos new position
 * @param supplement optional extra information
 *
 * @note this is the function that should be overridden in derived objects
 */
/*--------------------------------------------------------------------------------*/
void PositionEventRecorder::UpdatePositionEx(uint_t channel, const Position& pos, const ParameterSet *supplement)
{
  if (!file.isopen() && IsObjectValid() && (filename != ""))
  {
    if (!file.fopen(filename.c_str(), "w"))
    {
      ERROR("Failed to open file '%s' for writing", filename.c_str());
      InvalidateObject();
    }
  }

  UNUSED_PARAMETER(supplement);

  file.fprintf("%s\n", ControlEvent::ObjectName.c_str());
  if (timebase) file.fprintf("  time %0.6lf\n", timebase->GetTimeSeconds());
  file.fprintf("  control.object processor\n");
  file.fprintf("  control.control position\n");
  file.fprintf("  control.controlindex %u\n", channel);
  file.fprintf("  control.subaddr.1 %s\n",   pos.polar ? "polar" : "cart");
  file.fprintf("  control.value   %0.9le\n", pos.pos.x);
  file.fprintf("  control.value.1 %0.9le\n", pos.pos.y);
  file.fprintf("  control.value.2 %0.9le\n", pos.pos.z);
  file.fprintf("\n");
  file.fflush();

  SoundPositionConsumer::UpdatePositionEx(channel, pos, supplement);
}

BBC_AUDIOTOOLBOX_END
