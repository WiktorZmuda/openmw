#include "weather.hpp"
#include "world.hpp"

#include "../mwrender/renderingmanager.hpp"
#include "../mwsound/soundmanager.hpp"

#include <time.h>
#include <stdlib.h>

using namespace Ogre;
using namespace MWWorld;
using namespace MWSound;

#define TRANSITION_TIME 10

#define lerp(x, y) (x * (1-factor) + y * factor)

const std::string WeatherGlobals::mThunderSoundID0 = "Thunder0";
const std::string WeatherGlobals::mThunderSoundID1 = "Thunder1";
const std::string WeatherGlobals::mThunderSoundID2 = "Thunder2";
const std::string WeatherGlobals::mThunderSoundID3 = "Thunder3";

WeatherManager::WeatherManager(MWRender::RenderingManager* rendering, Environment* env) : 
     mHour(14), mCurrentWeather("clear"), mThunderFlash(0), mThunderChance(0), mThunderChanceNeeded(50),
     mThunderSoundDelay(0)
{
    mRendering = rendering;
    mEnvironment = env;
    
    #define clr(r,g,b) ColourValue(r/255.f, g/255.f, b/255.f)
    
    /// \todo read these from Morrowind.ini
    Weather clear;
    clear.mCloudTexture = "tx_sky_clear.dds";
    clear.mCloudsMaximumPercent = 1.0;
    clear.mTransitionDelta = 0.015;
    clear.mSkySunriseColor = clr(118, 141, 164);
    clear.mSkyDayColor = clr(95, 135, 203);
    clear.mSkySunsetColor = clr(56, 89, 129);
    clear.mSkyNightColor = clr(9, 10, 11);
    clear.mFogSunriseColor = clr(255, 189, 157);
    clear.mFogDayColor = clr(206, 227, 255);
    clear.mFogSunsetColor = clr(255, 189, 157);
    clear.mFogNightColor = clr(9, 10, 11);
    clear.mAmbientSunriseColor = clr(47, 66, 96);
    clear.mAmbientDayColor = clr(137, 140, 160);
    clear.mAmbientSunsetColor = clr(68, 75, 96);
    clear.mAmbientNightColor = clr(32, 35, 42);
    clear.mSunSunriseColor = clr(242, 159, 99);
    clear.mSunDayColor = clr(255, 252, 238);
    clear.mSunSunsetColor = clr(255, 115, 79);
    clear.mSunNightColor = clr(59, 97, 176);
    clear.mSunDiscSunsetColor = clr(255, 189, 157);
    clear.mLandFogDayDepth = 0.69;
    clear.mLandFogNightDepth = 0.69;
    clear.mWindSpeed = 0.1;
    clear.mCloudSpeed = 1.25;
    clear.mGlareView = 1.0;
    mWeatherSettings["clear"] = clear;
    
    Weather cloudy;
    cloudy.mCloudTexture = "tx_sky_cloudy.dds";
    cloudy.mCloudsMaximumPercent = 1.0;
    cloudy.mTransitionDelta = 0.015;
    cloudy.mSkySunriseColor = clr(126, 158, 173);
    cloudy.mSkyDayColor = clr(117, 160, 215);
    cloudy.mSkySunsetColor = clr(111, 114, 159);
    cloudy.mSkyNightColor = clr(9, 10, 11);
    cloudy.mFogSunriseColor = clr(255, 207, 149);
    cloudy.mFogDayColor = clr(245, 235, 224);
    cloudy.mFogSunsetColor = clr(255, 155, 106);
    cloudy.mFogNightColor = clr(9, 10, 11);
    cloudy.mAmbientSunriseColor = clr(66, 74, 87);
    cloudy.mAmbientDayColor = clr(137, 145, 160);
    cloudy.mAmbientSunsetColor = clr(71, 80, 92);
    cloudy.mAmbientNightColor = clr(32, 39, 54);
    cloudy.mSunSunriseColor = clr(241, 177, 99);
    cloudy.mSunDayColor = clr(255, 236, 221);
    cloudy.mSunSunsetColor = clr(255, 89, 00);
    cloudy.mSunNightColor = clr(77, 91, 124);
    cloudy.mSunDiscSunsetColor = clr(255, 202, 179);
    cloudy.mLandFogDayDepth = 0.72;
    cloudy.mLandFogNightDepth = 0.72;
    cloudy.mWindSpeed = 0.2;
    cloudy.mCloudSpeed = 2;
    cloudy.mGlareView = 1.0;
    mWeatherSettings["cloudy"] = cloudy;
    
    Weather thunderstorm;
    thunderstorm.mCloudTexture = "tx_sky_thunder.dds";
    thunderstorm.mCloudsMaximumPercent = 0.66;
    thunderstorm.mTransitionDelta = 0.03;
    thunderstorm.mSkySunriseColor = clr(35, 36, 39);
    thunderstorm.mSkyDayColor = clr(97, 104, 115);
    thunderstorm.mSkySunsetColor = clr(35, 36, 39);
    thunderstorm.mSkyNightColor = clr(19, 20, 22);
    thunderstorm.mFogSunriseColor = clr(70, 74, 85);
    thunderstorm.mFogDayColor = clr(97, 104, 115);
    thunderstorm.mFogSunsetColor = clr(70, 74, 85);
    thunderstorm.mFogNightColor = clr(19, 20, 22);
    thunderstorm.mAmbientSunriseColor = clr(54, 54, 54);
    thunderstorm.mAmbientDayColor = clr(90, 90, 90);
    thunderstorm.mAmbientSunsetColor = clr(54, 54, 54);
    thunderstorm.mAmbientNightColor = clr(49, 51, 54);
    thunderstorm.mSunSunriseColor = clr(91, 99, 122);
    thunderstorm.mSunDayColor = clr(138, 144, 155);
    thunderstorm.mSunSunsetColor = clr(96, 101, 117);
    thunderstorm.mSunNightColor = clr(55, 76, 110);
    thunderstorm.mSunDiscSunsetColor = clr(128, 128, 128);
    thunderstorm.mLandFogDayDepth = 1;
    thunderstorm.mLandFogNightDepth = 1.15;
    thunderstorm.mWindSpeed = 0.5;
    thunderstorm.mCloudSpeed = 3;
    thunderstorm.mGlareView = 0;
    mWeatherSettings["thunderstorm"] = thunderstorm;
    
    /*
    Weather overcast;
    overcast.mCloudTexture = "tx_sky_overcast.dds";
    overcast.mCloudsMaximumPercent = 1.0;
    mWeatherSettings["overcast"] = overcast;
    */
    
    setWeather("thunderstorm", true);
}

void WeatherManager::setWeather(const String& weather, bool instant)
{
    if (instant)
    {
        mNextWeather = "";
        mCurrentWeather = weather;
    }
    else if (weather != mCurrentWeather)
    {
        mNextWeather = weather;
        mRemainingTransitionTime = TRANSITION_TIME;
    }
}

WeatherResult WeatherManager::getResult(const String& weather)
{
    const Weather& current = mWeatherSettings[weather];
    WeatherResult result;
    
    result.mCloudTexture = current.mCloudTexture;
    result.mCloudBlendFactor = 0;
    result.mCloudOpacity = current.mCloudsMaximumPercent;
    result.mWindSpeed = current.mWindSpeed;
    result.mCloudSpeed = current.mCloudSpeed;
    result.mGlareView = current.mGlareView;
    result.mAmbientLoopSoundID = current.mAmbientLoopSoundID;
    
    const float fade_duration = 0.15 /*current.mTransitionDelta*/;
    
    result.mNight = (mHour < 6.f+fade_duration || mHour > 20.f-fade_duration);
    
    // night
    if (mHour <= (WeatherGlobals::mSunriseTime-WeatherGlobals::mSunriseDuration)
        || mHour >= (WeatherGlobals::mSunsetTime+WeatherGlobals::mSunsetDuration))
    {
        result.mFogColor = current.mFogNightColor;
        result.mAmbientColor = current.mAmbientNightColor;
        result.mSunColor = current.mSunNightColor;
        result.mSkyColor = current.mSkyNightColor;
        result.mNightFade = 1.f;
    }
    
    // sunrise
    else if (mHour >= (WeatherGlobals::mSunriseTime-WeatherGlobals::mSunriseDuration) && mHour <= WeatherGlobals::mSunriseTime)
    {
        if (mHour <= (WeatherGlobals::mSunriseTime-WeatherGlobals::mSunriseDuration+fade_duration))
        {
            // fade in
            float advance = (WeatherGlobals::mSunriseTime-WeatherGlobals::mSunriseDuration+fade_duration)-mHour;
            float factor = (advance / fade_duration);
            result.mFogColor = lerp(current.mFogSunriseColor, current.mFogNightColor);
            result.mAmbientColor = lerp(current.mAmbientSunriseColor, current.mAmbientNightColor);
            result.mSunColor = lerp(current.mSunSunriseColor, current.mSunNightColor);
            result.mSkyColor = lerp(current.mSkySunriseColor, current.mSkyNightColor);
            result.mNightFade = factor;
        }
        else if (mHour >= (WeatherGlobals::mSunriseTime-fade_duration))
        {
            // fade out
            float advance = mHour-(WeatherGlobals::mSunriseTime-fade_duration);
            float factor = advance / fade_duration;
            result.mFogColor = lerp(current.mFogSunriseColor, current.mFogDayColor);
            result.mAmbientColor = lerp(current.mAmbientSunriseColor, current.mAmbientDayColor);
            result.mSunColor = lerp(current.mSunSunriseColor, current.mSunDayColor);
            result.mSkyColor = lerp(current.mSkySunriseColor, current.mSkyDayColor);
        }
        else 
        {
            result.mFogColor = current.mFogSunriseColor;
            result.mAmbientColor = current.mAmbientSunriseColor;
            result.mSunColor = current.mSunSunriseColor;
            result.mSkyColor = current.mSkySunriseColor;
        }
    }
    
    // day
    else if (mHour >= (WeatherGlobals::mSunriseTime) && mHour <= (WeatherGlobals::mSunsetTime))
    {
        result.mFogColor = current.mFogDayColor;
        result.mAmbientColor = current.mAmbientDayColor;
        result.mSunColor = current.mSunDayColor;
        result.mSkyColor = current.mSkyDayColor;
    }
    
    // sunset
    else if (mHour >= (WeatherGlobals::mSunsetTime) && mHour <= (WeatherGlobals::mSunsetTime+WeatherGlobals::mSunsetDuration))
    {
        if (mHour <= (WeatherGlobals::mSunsetTime+fade_duration))
        {
            // fade in
            float advance = (WeatherGlobals::mSunsetTime+fade_duration)-mHour;
            float factor = (advance / fade_duration);
            result.mFogColor = lerp(current.mFogSunsetColor, current.mFogDayColor);
            result.mAmbientColor = lerp(current.mAmbientSunsetColor, current.mAmbientDayColor);
            result.mSunColor = lerp(current.mSunSunsetColor, current.mSunDayColor);
            result.mSkyColor = lerp(current.mSkySunsetColor, current.mSkyDayColor);
        }
        else if (mHour >= (WeatherGlobals::mSunsetTime+WeatherGlobals::mSunsetDuration-fade_duration))
        {
            // fade out
            float advance = mHour-(WeatherGlobals::mSunsetTime+WeatherGlobals::mSunsetDuration-fade_duration);
            float factor = advance / fade_duration;
            result.mFogColor = lerp(current.mFogSunsetColor, current.mFogNightColor);
            result.mAmbientColor = lerp(current.mAmbientSunsetColor, current.mAmbientNightColor);
            result.mSunColor = lerp(current.mSunSunsetColor, current.mSunNightColor);
            result.mSkyColor = lerp(current.mSkySunsetColor, current.mSkyNightColor);
            result.mNightFade = factor;
        }
        else 
        {
            result.mFogColor = current.mFogSunsetColor;
            result.mAmbientColor = current.mAmbientSunsetColor;
            result.mSunColor = current.mSunSunsetColor;
            result.mSkyColor = current.mSkySunsetColor;
        }
    }
    
    return result;
}

WeatherResult WeatherManager::transition(float factor)
{
    const WeatherResult& current = getResult(mCurrentWeather);
    const WeatherResult& other = getResult(mNextWeather);
    WeatherResult result;
    
    result.mCloudTexture = current.mCloudTexture;
    result.mNextCloudTexture = other.mCloudTexture;
    result.mCloudBlendFactor = factor;
    
    result.mCloudOpacity = lerp(current.mCloudOpacity, other.mCloudOpacity);
    result.mFogColor = lerp(current.mFogColor, other.mFogColor);
    result.mSunColor = lerp(current.mSunColor, other.mSunColor);
    result.mSkyColor = lerp(current.mSkyColor, other.mSkyColor);
    
    result.mAmbientColor = lerp(current.mAmbientColor, other.mAmbientColor);
    result.mSunDiscColor = lerp(current.mSunDiscColor, other.mSunDiscColor);
    result.mFogDepth = lerp(current.mFogDepth, other.mFogDepth);
    result.mWindSpeed = lerp(current.mWindSpeed, other.mWindSpeed);
    result.mCloudSpeed = lerp(current.mCloudSpeed, other.mCloudSpeed);
    result.mCloudOpacity = lerp(current.mCloudOpacity, other.mCloudOpacity);
    result.mGlareView = lerp(current.mGlareView, other.mGlareView);
    
    result.mNight = current.mNight;
    
    // sound change behaviour:
    // if 'other' has a new sound, switch to it after 1/2 of the transition length
    if (other.mAmbientLoopSoundID != "")
        result.mAmbientLoopSoundID = factor>0.5 ? other.mAmbientLoopSoundID : current.mAmbientLoopSoundID;
    // if 'current' has a sound and 'other' does not have a sound, turn off the sound immediately
    else if (current.mAmbientLoopSoundID != "")
        result.mAmbientLoopSoundID = "";
        
    return result;
}

void WeatherManager::update(float duration)
{
    if (mEnvironment->mWorld->isCellExterior() || mEnvironment->mWorld->isCellQuasiExterior())
    {
        WeatherResult result;
        
        if (mNextWeather != "")
        {
            mRemainingTransitionTime -= duration;
            if (mRemainingTransitionTime < 0)
            {
                mCurrentWeather = mNextWeather;
                mNextWeather = "";
            }
        }
        
        if (mNextWeather != "")
            result = transition(1-(mRemainingTransitionTime/TRANSITION_TIME));
        else
            result = getResult(mCurrentWeather);
        
        // disable sun during night
        if (mHour >= WeatherGlobals::mSunsetTime+WeatherGlobals::mSunsetDuration
            || mHour <= WeatherGlobals::mSunriseTime-WeatherGlobals::mSunriseDuration)
            mRendering->getSkyManager()->sunDisable();
        else
        {
            // during day, calculate sun angle
            float height = 1-std::abs(((mHour-13)/7.f));
            int facing = mHour > 13.f ? 1 : -1;
            Vector3 final(
                (1-height)*facing, 
                (1-height)*facing, 
                height);
            mRendering->setSunDirection(final);
            
            mRendering->getSkyManager()->sunEnable();
        }
        
        if (mCurrentWeather == "thunderstorm" && mNextWeather == "")
        {
            if (mThunderFlash > 0)
            {
                // play the sound after a delay
                mThunderSoundDelay -= duration;
                if (mThunderSoundDelay <= 0)
                {
                    // pick a random sound
                    int sound = rand() % 4;
                    std::string soundname;
                    if (sound == 0) soundname = WeatherGlobals::mThunderSoundID0;
                    else if (sound == 1) soundname = WeatherGlobals::mThunderSoundID1;
                    else if (sound == 2) soundname = WeatherGlobals::mThunderSoundID2;
                    else if (sound == 3) soundname = WeatherGlobals::mThunderSoundID3;
                    #include <iostream>
                    std::cout << "play sound" << std::endl;
                    mEnvironment->mSoundManager->playSound(soundname, 1.0, 1.0);
                    mThunderSoundDelay = 1000;
                }
                
                mThunderFlash -= duration;
                if (mThunderFlash > 0)
                    mRendering->getSkyManager()->setThunder( mThunderFlash / WeatherGlobals::mThunderThreshold );
                else
                {
                    srand(time(NULL));
                    mThunderChanceNeeded = rand() % 100;
                    mThunderChance = 0;
                    mRendering->getSkyManager()->setThunder( 0.f );
                }
            }
            else
            {
                // no thunder active
                mThunderChance += duration*4; // chance increases by 4 percent every second
                if (mThunderChance >= mThunderChanceNeeded)
                {
                    mThunderFlash = WeatherGlobals::mThunderThreshold;
                    
                    mRendering->getSkyManager()->setThunder( mThunderFlash / WeatherGlobals::mThunderThreshold );
                    
                    mThunderSoundDelay = WeatherGlobals::mThunderSoundDelay;
                }
            }
        }
        
        mRendering->setAmbientColour(result.mAmbientColor);
        mRendering->sunEnable();
        mRendering->setSunColour(result.mSunColor);
        
        mRendering->skyEnable();
        mRendering->getSkyManager()->setWeather(result);
    }
    else
    {
        mRendering->sunDisable();
        mRendering->skyDisable();
        mRendering->getSkyManager()->setThunder(0.f);
    }
}

void WeatherManager::setHour(const float hour)
{
    // accelerate a bit for testing
    /*
    mHour += 0.005;
    
    if (mHour >= 24.f) mHour = 0.f;
    
    #include <iostream>
    std::cout << "hour " << mHour << std::endl;
    */
    
    mHour = hour;
}

void WeatherManager::setDate(const int day, const int month)
{
    mDay = day;
    mMonth = month;
}
