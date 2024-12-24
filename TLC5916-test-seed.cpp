#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;

#define PIN_DATA 1
#define PIN_CLK 2
#define PIN_LATCH 3

dsy_gpio dataGpio;
dsy_gpio clockGpio;
dsy_gpio latchGpio;

Metro tick;
int clkState = 0;
int clkCount = 0;
int tickCount = 0;
int flip = 0;
uint8_t isTick = 0;
int value = 0;

int ledTempState[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int ledState[8] = {0, 0, 0, 0, 0, 1, 1, 1};

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
  for (size_t i = 0; i < size; i++)
  {
    out[0][i] = in[0][i];
    out[1][i] = in[1][i];
    isTick = tick.Process();

    if (isTick)
    {
      ledTempState[tickCount] = !ledTempState[tickCount];
      tickCount = (tickCount + 1) % 8;

      if (tickCount % 2 == 0)
      {
        clkState = !clkState;
      }

      if (!clkState)
      {
        dsy_gpio_write(&dataGpio, (value >> clkCount) & 1);
      }

      if (tickCount % 2 == 0)
      {
        if (clkState)
        {
          if (clkCount == 7)
          {
            dsy_gpio_write(&latchGpio, 1);
            value = (value + 1) % 256;
          }
          clkCount = (clkCount + 1) % 8;
        }
        else
        {
          dsy_gpio_write(&latchGpio, 0);
          dsy_gpio_write(&dataGpio, 0);
        }
      }

      dsy_gpio_write(&clockGpio, clkState);
    }
  }
}

int main(void)
{
  hw.Init();

  float sampleRate = hw.AudioSampleRate();

  hw.StartLog();
  System::Delay(100);

  tick.Init(1000, sampleRate);

  dataGpio.pin = hw.GetPin(PIN_DATA);
  dataGpio.mode = DSY_GPIO_MODE_OUTPUT_PP;
  dataGpio.pull = DSY_GPIO_NOPULL;
  dsy_gpio_init(&dataGpio);

  clockGpio.pin = hw.GetPin(PIN_CLK);
  clockGpio.mode = DSY_GPIO_MODE_OUTPUT_PP;
  clockGpio.pull = DSY_GPIO_NOPULL;
  dsy_gpio_init(&clockGpio);

  latchGpio.pin = hw.GetPin(PIN_LATCH);
  latchGpio.mode = DSY_GPIO_MODE_OUTPUT_PP;
  latchGpio.pull = DSY_GPIO_NOPULL;
  dsy_gpio_init(&latchGpio);

  hw.SetAudioBlockSize(4); // number of samples handled per callback
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
  hw.StartAudio(AudioCallback);
  while (1)
  {
    // if(isTick) {
    //   hw.PrintLine("clkCount: %d", clkCount);
    // }
  }
}
