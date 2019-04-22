# Project Aim of - Kitchen Senor Array
## Project Goal: 
1. add senor in the kitchen to allow Home Assistant to process all automation objectives
2. must be completely reversible without leaving damage - as I rent

### Primary Objectives:
* track room light level - unfortunately, I'm not able to make my kitchen light smart. So best can do here is alert me, if left on and I go bed or out.
* monitor the dishwasher status - on completion, Alexa to say "dishwasher done", plus notification on the phone
* monitor the washer status - on completion, Alexa to say "washer done", plus notification on the phone  

### Secondary Objectives:
* monitor dishwasher door - if no once unloaded after x minutes, send a reminder
* monitor washer door - if no once unloaded after x minutes, send a reminder  
* track room temperature  & humidity - it's nice to see how this change throughout the house room by room. Also might be able to track if someone cooking
* motion detection - add to the alarm grid
* back door status - detect if the backdoor is left open,  alert me, if left on and I go bed or out.
* window status - detect if the backdoor is left open,  alert me, if left on and I go bed or out.  


# Plan
- Prototype Circuit 
    -  make on breadboard
    -  create an Arduino sketch, to post senor to MQTT
- 3D Print Senor Case
    - Motion sensor case - I have stl file design I used before
    - DHT11 with LDR - have an existing design for this
    - Dishwasher LDR holder - this will be new and very custom to this model