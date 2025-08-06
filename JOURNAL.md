---
title: "slider synth"
author: "@Danny"
description: "A esp32 based synth that uses a touch pot and fsr."
created_at: "2025-07-23"
---

**Total time take: 15 hours**

# June 23rd: Planning + Schematic

<img width="816" height="539" alt="image" src="https://github.com/user-attachments/assets/b5cc40a0-ecc6-4740-b3f3-5d73176a3d98" />

I used some of my own references as well as these ones below. I was able to setup a lot of essential features like usb c flashing, 5v to 3v3, uart, and boot/reset buttons.
* [devkit schematics](https://dl.espressif.com/dl/schematics/SCH_ESP32-S3-DevKitC-1_V1.1_20220413.pdf)
* [esp32-s3-wroom datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf)
* [user guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html)

**Total time spent: ~4 hours**

# June 24th: Adding more features

<img width="540" height="595" alt="image" src="https://github.com/user-attachments/assets/b751e54f-aefb-477d-86c2-686868a9fd39" />

I finished all of the audio circuit above and added more features like led indicators and a rotary encoder for extra control.

<img width="384" height="503" alt="image" src="https://github.com/user-attachments/assets/17b0ad3d-66af-46db-918b-e85c2bb723a0" />

**Total time spent: ~6 hours**

# June 25th: Placement, Routing, CAD, and everything else

<img width="641" height="401" alt="image" src="https://github.com/user-attachments/assets/32758920-a8ff-4a93-8d3d-b2fb3bbdd001" />

For placement, I had a solid plan of selecting an entire box/feature in my schematic and seperating that to then place.

After everything was placed, I can finally route and rotate any components if needed for better routing.

<img width="835" height="375" alt="image" src="https://github.com/user-attachments/assets/e124b456-bb7d-42e7-ab4d-b9a1bf4dc651" />

I made a simple case that would press fit onto my plywood strip and pcb. I wanted it to be flush with the thickness of the plywood to not add any extra height.

**Total time spent: ~5 hours**

# MILESTONE: MY PROJECT WAS APPROVED!!!!

Time to wait for the grant and everything to be shipped.

**Total time spent: ~0 hours**

# August 4th: PCB Assembly

<img width="621" height="733" alt="image" src="https://github.com/user-attachments/assets/00805163-b322-4884-9d90-6cba311df5d0" />

My pcbs arrived! I chose to not to include some parts as it would cost more to assemble. I have some on hand already, like the esp32-s3-wroom.

<img width="457" height="287" alt="image" src="https://github.com/user-attachments/assets/dc5a8d46-7eb9-4951-b020-8edae318cf85" />

I also did not order a stencil, so I had to manually apply and shape the solder paste to each pad. It was okay.

**Total time spent: ~4 hours**

# August 5th: Finished Assembly, now Debugging

<img width="660" height="595" alt="image" src="https://github.com/user-attachments/assets/986b5f75-e1a2-4412-9547-9884ed173d0d" />

<img width="678" height="547" alt="image" src="https://github.com/user-attachments/assets/429bfbf4-8c96-40bd-b355-b3eed6b51be4" />

I am so glad I connected my UART to these headers for debugging on my oscilliscope. For some reason, I can't get platformio to read any serial.

<img width="576" height="547" alt="image" src="https://github.com/user-attachments/assets/4133c6a4-772d-4232-a5c7-ca722a7c1a8c" />

I am so stupid. I did not check if the pinout i chose for my force sensitive resistor supported adc. (IO38) I had to solder a jumper wire to fix this issue.

Moving onto the code part, something is not right. I am using mozzi which is a dsp library for synths. For some reason, it either outputs nothing or strange frequencies. Hopefully its not my schematic that is wrong.

**Total time spent: ~6 hours**

