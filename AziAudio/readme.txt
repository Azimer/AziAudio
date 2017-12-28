**************************************
*** Azimer's HLE Audio v0.70 WIP 1 ***
**************************************

Last Updated: July 28th, 2014


----- Contents -----

0. Disclaimer
1. What's New
2. Known Issues
3. What's Next
4. Usage
5. FAQ
6. Greetings and Closing Comments

----------------------------------------------------------------------------------

0. Disclaimer

This plugin is provided as is for beta testing purposes only.  You are
solely responsible for any damages or data loss incurred through use of
this software.  Use at your own risk.  If you do not agree with these
terms, please delete this application and use something else.  The software
has been tested to the best of my ability.  There is no warranty expressed
or implied.  Commercial use of this product will constitute a criminal act.
blah blah blah... if you can think of an illegitimate use for this product,
please do not use it.  I am not responsible for stupidity. Now that is over
with...

----------------------------------------------------------------------------------

1. What's New?

*** v0.70 WIP 1 ***
- Implemented XAudio2 with its own audio code.  Use for best results.
- Updated DirectSound code to better support Project64 2.1
- Fixed issue where if there's no audio initialized it will throw errors.  Now it
  it just won't do anything at all.
- Fixed Mystical Ninja pops
- Reduced Smash Bros crackles a little bit - still some issues
- Stability changes (less crashing I hope)
- Killed the smilies from this document

*** v0.60 WIP 2 ***
- New audio code - tested in Project64 1.6
- New configuration though most things are disabled atm

*** v0.56 WIP 2 ***
- Bug Fix release... nothing much new
- Finally received Project64 1.6
- Hydro Thunder and Tarzan work well by using Project64 1.6's 
- Fixed those nasty SP_DMA_READ Errors
- Fixed hangups in all roms which worked before
- Eliminated some pops and crackles in games that shouldn't have them
- And no... configuration has not been changed (least of my worries)

*** v0.56 WIP 1 ***
- Revamped Audio Code from v0.50.2
- HLE Code is untouched as of now
- Added some low level RSP code temporarily to assist in fixing HLE Audio
- Added a hidden console window for Audio status/debugging (disabled unless public finds it desirable)
- Fixed MANY stuttering issues and issues with MusyX games.  Tarzan is still a problem because of timing.
- Added Old Audio Sync option... You will need to set it every time you start the emulator if you want to use it.

----------------------------------------------------------------------------------
2. Known Issues (possibly unfixable within the confines of this plugin spec)
- Cruis'n World - Audio is too fast without emulator's audio fix and sync enabled
- Twisted Edge - No audio (Emulator bug??)
- Big Boss games - Stuntracer, Top Gear Rally, World Driver Championship, Twisted Edge 
	- Still a huge issue with audio quality... I believe it's an emulator issue
- Indiana Jones - Factor 5 intro sequence is very buggy

----------------------------------------------------------------------------------

3. What's Next?
*** Coming Soon ***
- Merge DirectSound and XAudio2 code into one plugin again
- Fix Golden Eye HLE Envelope Mixer code
- Enabled saving audio config settings
- Add MusyX HLE Code

*** Near Future ***
- More audio configuration options (Buffer size adjustments, Bitrate adjustments, MP3 logging)
- Equalizer/Filters
- HLE speedups/optimizations

*** Maybes ***
- Find a solution to decode Dolby Prologic
- Music isolation for MP3 logging (big maybe)

----------------------------------------------------------------------------------

4. Usage

*** Setup ***
1. Place AziAudioDS8.dll and AziAudioXA2.dll into the "Project64" folder's "Plugin" folder. (ie C:\Project64 2.1\Plugin\Audio)
2. Go to the Options menu, click settings.
3. Go to the Plugins setting tab.
4. Under "Audio (sound) plugin click the drop down box.
5. Select "Azimer's XA2 Audio" whatever version is the latest (ie "Azimer's XA2 Audio v0.70 WIP 1")
6. All the default options should be okay, so click ok

You should now be good to load an image as defined in the Project64 user help file.

----------------------------------------------------------------------------------

5. FAQ

5.1.Q - Why do I hear some pops in some parts of the game?
5.1.A - I have noticed through my own testing that during certain parts of a game, the game uses the
        emulated processor power to the point where it can not perform at a constant 60/50FPS if even
        for a moment.  This neglect will cause the constant stream of audio to the plugin to be halted
        for that brief popping period.  It is not a problem with the Audio plugin and nothing can be
        done to accommodate.  You can try setting the emulator to High priority to eliminate the turn
        around time for the emulator to preempt any process during its waitstate.  It seems to have
        helped me.  Another problem is that PJ64 1.5's speed limiter isn't the best.  If it's a compatible
        game, you could attempt to use the old speed sync.  A better speed fix is underway to go by samples
        per second based on the set frequency.  This is likely another audio code revision just to try it out.

5.2.Q - Why isn't Tarzan allowing me to go in game?
5.2.A - This problem has finally be isolated to an option acquired in Project64 1.6.  Make sure you have
        RSP Audio Signal checked in the Rom Settings options.

5.3.Q - Is there going to be another Project64 Audio Fix?
5.3.A - Nothing is planned.  I'd rather contribute directly to the source but if a release is far off,
        it's a possibility.

5.4.Q - How come there's no audio in Tarzan, Hydro Thunder, TWINE, or any other MusyX game?
5.4.A - Try changing the options in the Emulator so Audio Lists are not processed by Audio HLE.  Musyx is not
        currently supported.

5.5.A - There seems to be problems in a game I've been playing?  How should I contact you?
5.5.Q - Please post all problems on EmuTalk's Apollo board.  I will be more than happy to answer any questions.

5.6.A - I am getting an SP_READ_DMA Error?  What does this mean?
5.6.Q - Either you are using an older version of this plugin or you have encountered a ROM I haven't tested.
        Please write me on emutalk with the specific GoodN64 rom title and I will look into it. Thanks.

If none of those FAQs help you, please post on EmuTalk and I will answer the question and include it here.
www.emutalk.net

----------------------------------------------------------------------------------

6. Greetings and Closing Comments

I would like to thank the following for all their help.
RCP     - For giving me a booster for HLE Audio.
F|RES   - Thanks for allowing me to join TR64 years ago and getting a start on Audio HLE from this.
icepir8 - If you ever continue TR64, feel free to include my Audio plugin.  It will always be a TR64 plugin.
zilmar  - The single biggest help with AudioHLE.  Thank you for all the RSP help and information.  It was invaluable.
Jabo    - I still use a variant of you Audio playing idea you proposed to be years ago from JNes.  Thanks!
LaC     - Many long conversations that now make me feel newbish.  Thanks for putting up with me.

Hack, Rice, Schibo, Phrodide, episilon, realityman, and everyone else who has worked on N64 emulation.  You all
have contributed to the success of our interest.  Also like to say thanks to breakpoint for pioneering N64 
emulation.  I will never forget the days I waited to see better screenshots of MKT.  I was very sad when you
stopped.

Special thanks to all my past beta testers and supporters.  Thank you to olivieryuyu who maintained the Audio FAQ in
the Apollo board.  Your contribution has been tremendously helpful.  Thank you to those who inspired me to 
continue working on new releases.  It is not an easy thing to pick up again after years of non-work and a family
to support.  Thank you to all those who I have forgotten, it wasn't intentional.

Though N64 emulation seems to have died out from it's golden age, there is still a lot of work that needs to be
done.

-Azimer (www.apollo64.com)
