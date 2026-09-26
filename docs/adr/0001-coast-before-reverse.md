# Coast Before Reverse on Border Detection

Yellow TT gearmotors create substantial inductive back-EMF and current spikes when abruptly reversed at full speed. We insert a brief coasting interval (30–50 ms) upon Border detection before applying reverse thrust, preventing Arduino Nano supply rail brownouts and mechanical gear shock at the cost of slight initial roll.
