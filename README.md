For more info about ReShade, go to https://reshade.me/.

This aims at removing the fog in Gw2, but I decided to add more functions to it. This at still in an early stage, and should be used keeping this in mind.  Also, I’ve never used DirectX nor any low-level Graphic API and rarely used C++, the code is far from being clean!
So, to remove the fog, I installed a hook on the CreatePixelShader, so I have access to each shader’s bytecode. After some researches, I found a pattern in the bytecode that is used to apply the fog, so basically I just had to change a mad (multiply and add) op that applied the final tex to the object with the fog to a mov (which… move the tex, without adding the fog).
To skip the HUD, I removed the hook in on_present that applied the shaders, installed two hook instead:
One in CreateVertexShader, that allow me to search and save the ref of the shader after which I want to apply the FXs.
One in SetVertexArray, that call FXs to be draw if the shader is the one I saved the ref before.
