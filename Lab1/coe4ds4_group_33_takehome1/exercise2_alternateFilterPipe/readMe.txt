Only difference is the implementation approache for Switch 7 or Filter_Config[2:0] == 3'd7.
Alternate Approach Used falling Vsync edge of (2'b10) to keep count of a frame. 60 of these Vsync edges meant 1 second of count. During the 60th Vsync edge, would update the image. This implementation correctly cycles through first four images for Filter_Config[2:0] == 3'd7.
