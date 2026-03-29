# Post Fabrication Results and Testing

After soldering everything and writing firmware to the slider-synth, there were a couple issues that lowered its quality in general.

At first, one of the sensors did not work at all because I accidentally used a gpio that didn't support analog input. I solved this by soldering a jumper wire which sticks out a little bit, but could easily be fixed for a second round of pcbs.         
<img width="204" height="229" alt="image" src="https://github.com/user-attachments/assets/5202e742-c738-47db-8acf-eb910555afd5" />           

The audio output kind of sucks. Although it does give a nice harsh analog sound, it is really irritating and no code can fix it due to the passives/ics I used for audio out. I could definitely retry a better design.          
   
<img width="426" height="361" alt="image" src="https://github.com/user-attachments/assets/e5bae18c-8f55-4c24-af41-6c6d0f3ae28d" />     

Plus, i was a little confused and the library i use for digital signal processing (mozzi) specified something else different for my audio. I was trying to implement its [hi-fi audio](https://sensorium.github.io/Mozzi/learn/output/) but accidentally muted the right channel making it a mono output


The PCB also lacks some real mounting holes for more professional case/mounts, and the press fit design sometimes fails due to force from the controls
<img width="1167" height="380" alt="image" src="https://github.com/user-attachments/assets/4abad88a-7dea-41a0-b871-0142d77a74a7" />         
