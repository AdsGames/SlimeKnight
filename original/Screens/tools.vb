Imports System.Collections.Generic
Imports System.Linq
Imports Microsoft.Xna.Framework
Imports Microsoft.Xna.Framework.Audio
Imports Microsoft.Xna.Framework.Content
Imports Microsoft.Xna.Framework.GamerServices
Imports Microsoft.Xna.Framework.Graphics
Imports Microsoft.Xna.Framework.Input
Imports Microsoft.Xna.Framework.Media
Imports FlatRedBall
Imports FlatRedBall.Graphics
Imports FlatRedBall.Utilities

Imports ConsoleApplication1.Screens
Imports FlatRedBall.Input
Imports FlatRedBall.AI.Pathfinding
Imports FlatRedBall.Graphics.Animation
Imports FlatRedBall.Graphics.Particle

Imports FlatRedBall.Graphics.Model
Imports FlatRedBall.Math.Geometry
Imports FlatRedBall.Math.Splines

Imports Cursor = FlatRedBall.Gui.Cursor
Imports GuiManager = FlatRedBall.Gui.GuiManager
Imports FlatRedBall.Localization
Imports FlatRedBall.Gui
Imports FlatRedBall.IO
#If FRB_XNA OrElse SILVERLIGHT Then
Imports Keys = Microsoft.Xna.Framework.Input.Keys
Imports Vector3 = Microsoft.Xna.Framework.Vector3
Imports Texture2D = Microsoft.Xna.Framework.Graphics.Texture2D
#End If
Public Class tools
    Public scene As Scene


    Public Function loadScene(sceneFileName As String, contentManagerName As String) As Scene
        'the following three lines add the scene designed in the SpriteEditor into your screen
        scene = FlatRedBallServices.Load(Of Scene)(sceneFileName, contentManagerName)
        scene.AddToManagers()
        Return scene
    End Function

    Public Function loadSprite(spriteEditorName As String) As Sprite
        Return scene.Sprites.FindByName(spriteEditorName)
    End Function

    Public Function changeTexture(ByVal fileName As String) As Texture
        Return FlatRedBallServices.Load(Of Texture2D)(fileName)
    End Function

    Public Function loadText(spriteEditorName As String) As Text
        Return scene.Texts.FindByName(spriteEditorName)
    End Function

    Public Function loadSoundEffect(fileName As String) As SoundEffect
        Return FlatRedBallServices.Load(Of SoundEffect)(fileName)
    End Function

    Public Sub mapCollisionRectangle(forSprite As Sprite)
        Dim rectangle As FlatRedBall.Math.Geometry.AxisAlignedRectangle = New FlatRedBall.Math.Geometry.AxisAlignedRectangle()
        rectangle.ScaleX = forSprite.ScaleX
        rectangle.ScaleY = forSprite.ScaleY
        forSprite.SetCollision(rectangle)
    End Sub

    Public Sub moveToScreen(screenName As Screen)
        'moveToScreen(screenName)
    End Sub

    Public Function isKeyPressed(key As Keys) As Boolean
        Return FlatRedBall.Input.InputManager.Keyboard.KeyDown(key)
    End Function

    Public Function isKeyPushed(ByVal key As Keys) As Boolean
        Return FlatRedBall.Input.InputManager.Keyboard.KeyPushed(key)
    End Function

    Public Function getMouseX() As Single
        Return InputManager.Mouse.WorldXAt(0)
    End Function

    Public Function getMouseY() As Single
        Return InputManager.Mouse.WorldYAt(0)
    End Function

    Public Function isLeftMouseClicked() As Boolean
        Return InputManager.Mouse.ButtonPushed(FlatRedBall.Input.Mouse.MouseButtons.LeftButton)
    End Function

    Public Function isRightMouseClicked() As Boolean
        Return InputManager.Mouse.ButtonPushed(FlatRedBall.Input.Mouse.MouseButtons.RightButton)
    End Function

    Public Sub exitGame()
        FlatRedBallServices.Game.Exit()
    End Sub

    Public Function loadFile(fileName As String) As saveData
        Return FileManager.XmlDeserialize(Of saveData)(fileName)
    End Function

    Public Sub saveFile(saveDataObject As saveData, fileName As String)
        'FileManager.BinarySerialize(saveDataObject, fileName)()
    End Sub

    Public Function getSound(fileName As String) As SoundEffect
        Return FlatRedBallServices.Load(Of SoundEffect)(fileName)
    End Function

    Public Function random(ByVal firstInt As Integer, ByVal secondInt As Integer) As Integer
        Return FlatRedBallServices.Random.Next(firstInt, secondInt)
    End Function
End Class
