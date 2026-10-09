from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import pyautogui

#Our app
app = FastAPI()

#Puts the middleware in
app.add_middleware(CORSMiddleware,
                   allow_origins=["192.168.1.238"],
                   allow_credentials=True,
                   allow_methods=["*"],
                   allow_headers=["*"])

#Message
class Message(BaseModel):
    text: str
    
#X and Y
class XY(BaseModel):
    x: int
    y: int

#Click
class Click(BaseModel):
    click: bool
    
#Click
@app.post("/clickControl")
def click(wasClicked : Click):
    if wasClicked.click:
        pyautogui.click()
        
#Gets the information
@app.post("/remoteControl")
def message(message : Message):
    command = message.text
    print(command)
    if command == "_":
        pyautogui.press("enter")
    else:
        pyautogui.write(message.text)
        pyautogui.write(" ")
    
#Gets the x and y coordinates
@app.post("/X_And_Y")
def getXAndY(xAndY : XY):
    #print(xAndY.x)
    #print(xAndY.y)
    #Moves with the potenmenter
    pyautogui.moveTo(xAndY.x / 2, xAndY.y / 2,duration=1)