
#include <stdio.h>
#include <stdlib.h>

#include <bbcat-base/LoadedVersions.h>

#include <bbcat-audioobjects/ADMData.h>
#include <bbcat-audioobjects/XMLADMData.h>

using namespace bbcat;

// ensure the version numbers of the linked libraries and registered
BBC_AUDIOTOOLBOX_REQUIRE(bbcat_base_version);
BBC_AUDIOTOOLBOX_REQUIRE(bbcat_dsp_version);
BBC_AUDIOTOOLBOX_REQUIRE(bbcat_control_version);
BBC_AUDIOTOOLBOX_REQUIRE(bbcat_audioobjects_version);

// ensure the TinyXMLADMData object file is kept in the application
BBC_AUDIOTOOLBOX_REQUIRE(TinyXMLADMData);

int main(void)
{
  // print library versions (the actual loaded versions, if dynamically linked)
  printf("Versions:\n%s\n", LoadedVersions::Get().GetVersionsList().c_str());

  XMLADMData *adm;
  
  // create basic ADM
  if ((adm = XMLADMData::CreateADM()) != NULL)
  {
    ADMData::OBJECTNAMES names;

    // set programme name
    // if an audioProgramme object of this name doesn't exist, one will be created
    names.programmeName = "ADM Test Programme";

    // set content name
    // if an audioContent object of this name doesn't exist, one will be created
    names.contentName   = "ADM Test Content";
    
    // create 16 tracks, channels and streams
    uint_t t, ntracks = 16;
    for (t = 0; t < ntracks; t++)
    {
      std::string trackname;

      printf("------------- Track %2u -----------------\n", t + 1);

      // create default audioTrackFormat name (used for audioStreamFormat objects as well)
      Printf(trackname, "Track %u", t + 1);

      names.trackNumber = t;

      // derive channel and stream names from track name
      names.channelFormatName = trackname;
      names.streamFormatName  = "PCM_" + trackname;
      names.trackFormatName   = "PCM_" + trackname;

      // set object name
      // create 4 objects, each of 4 tracks
      names.objectName = "";  // need this because Printf() APPENDS!
      Printf(names.objectName, "Object %u", 1 + (t / 4));
        
      // set pack name from object name
      // create 4 packs, each of 4 tracks
      names.packFormatName = "";  // need this because Printf() APPENDS!
      Printf(names.packFormatName, "Pack %u", 1 + (t / 4));

      adm->CreateObjects(names);

      // note how the programme and content names are left in place in 'names'
      // this is necessary to ensure that things are linked up properly

      // find channel format object for this track
      ADMAudioChannelFormat *cf;
      if ((cf = dynamic_cast<ADMAudioChannelFormat *>(adm->GetWritableObjectByName(names.channelFormatName, ADMAudioChannelFormat::Type))) != NULL)
      {
        // found channel format, generate block formats for it
        ADMAudioBlockFormat   *bf;
        uint_t i;

        for (i = 0; i < 20; i++)
        {
          if ((bf = adm->CreateBlockFormat(cf)) != NULL)
          {
            AudioObjectParameters params;
            Position pos;
              
            // set start time to be index * 25ms
            bf->SetRTime(i * 25000000);
            // set duration to be 25ms
            bf->SetDuration(25000000);

            pos.polar  = true;
            pos.pos.az = fmod((double)(t + i) * 20.0, 360.0);
            pos.pos.el = (double)i / (double)ntracks * 60.0;
            pos.pos.d  = 1.0;
            params.SetPosition(pos);

            params.SetGain(2.0);
            params.SetWidth(5.0);
            params.SetHeight(10.0);
            params.SetDepth(15.0);
            params.SetDiffuseness(20.0);
            params.SetDelay(25.0);
            params.SetObjectImportance(5);
            params.SetChannelImportance(2);
            params.SetDialogue(1);
            params.SetChannelLock(true);
            params.SetInteract(true);
            params.SetInterpolate(true);
            params.SetInterpolationTimeS(5.2);
            params.SetOnScreen(true);
            ParameterSet othervalues = params.GetOtherValues();
            params.SetOtherValues(othervalues.Set("other1", 1).Set("other2", "2"));

            // set object parameters
            bf->GetObjectParameters() = params;
          }
          else fprintf(stderr, "Failed to create audioBlockFormat (t: %u, i: %u)\n", t, i);
        }
      }
      else
      {
        // this will only occur in the case of an error
        fprintf(stderr, "Unable to find channel format '%s'\n", names.channelFormatName.c_str());
      }
    }

    // finalise ADM
    adm->Finalise();
    
    // output ADM
    printf("XML:\n%s", adm->GetAxml().c_str());
  }

  return 0;
}
