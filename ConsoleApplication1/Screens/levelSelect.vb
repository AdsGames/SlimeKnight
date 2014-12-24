'Allan Legemaate
'1/22/13
'Level Select
'Choose a level
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

Imports NewFlatRedBallProject.Screens
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

#If FRB_XNA OrElse SILVERLIGHT Then
Imports Keys = Microsoft.Xna.Framework.Input.Keys
Imports Vector3 = Microsoft.Xna.Framework.Vector3
Imports Texture2D = Microsoft.Xna.Framework.Graphics.Texture2D
#End If

Namespace Screens
    Partial Public Class levelSelect
        'Create scene
        Dim level1 As Scene

        'Create a variable to hold the tools object
        Dim tool As New tools

        'Setup sprites
        Dim level_1 As Sprite
        Dim level_2 As Sprite
        Dim level_3 As Sprite
        Dim frame As Sprite
        Dim levelSelect As Sprite
        Dim level As Integer
        Dim cursor As Sprite
        Dim back As Sprite

        'Setup sound effects
        Dim click As SoundEffect
        Dim music As Song


        Private Sub CustomInitialize()
            'load the scene
            level1 = tool.loadScene("Content\\levelSelect.scnx", Me.ContentManagerName)

            'load sprites
            level_1 = tool.loadSprite("level1")
            level_2 = tool.loadSprite("level2")
            level_3 = tool.loadSprite("level3")
            frame = tool.loadSprite("frame")
            levelSelect = tool.loadSprite("levelSelect")
            back = tool.loadSprite("back")
            back.ScaleX = 4
            back.ScaleY = 1.5
            cursor = SpriteManager.AddSprite("Content\\images\\cursor.PNG")
            cursor.Z = 10
            tool.mapCollisionRectangle(cursor)
            tool.mapCollisionRectangle(level_1)
            tool.mapCollisionRectangle(level_2)
            tool.mapCollisionRectangle(level_3)
            tool.mapCollisionRectangle(back)

            'level integer
            level = 1

            'Load sound effects
            click = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\click", ContentManagerName)

            'Load music
            music = FlatRedBallServices.Load(Of Song)("Content\\sounds\\menu", ContentManagerName)
            Microsoft.Xna.Framework.Media.MediaPlayer.Play(music)
            MediaPlayer.IsRepeating = True
        End Sub

        Private Sub CustomActivity(ByVal firstTimeCalled As Boolean)

            'Mouse routines
            cursor.X = tool.getMouseX
            cursor.Y = tool.getMouseY

            'Select Menu Componants
            If cursor.CollideAgainst(level_1) And level <> 1 Then
                frame.Visible = True
                frame.X = -14
                levelSelect.Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\levelSelect_level1.png")
                level = 1
                click.Play()
            ElseIf cursor.CollideAgainst(level_2) And level <> 2 Then
                frame.Visible = True
                frame.X = 0
                levelSelect.Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\levelSelect_level2.png")
                level = 2
                click.Play()
            ElseIf cursor.CollideAgainst(level_3) And level <> 3 Then
                frame.Visible = True
                frame.X = 14
                levelSelect.Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\levelSelect_level3.png")
                level = 3
                click.Play()
            ElseIf cursor.CollideAgainst(back) Then
                frame.Visible = False
                back.Texture = tool.changeTexture("Content\\images\\buttons\\back_hover.png")
                back.ScaleX = 4
                back.ScaleY = 1.5
                levelSelect.Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\levelSelect_choose.png")
                level = 4
            ElseIf cursor.CollideAgainst(back) = False Then
                back.Texture = tool.changeTexture("Content\\images\\buttons\\back.png")
                back.ScaleX = 4
                back.ScaleY = 1.5
            End If

            'Load level
            If InputManager.Mouse.ButtonPushed(FlatRedBall.Input.Mouse.MouseButtons.LeftButton) Then
                CustomDestroy()
                If level = 4 Then
                    MoveToScreen(GetType(menu).FullName)
                ElseIf level = 1 Then
                    MoveToScreen(GetType(level1).FullName)
                ElseIf level = 2 Then
                    MoveToScreen(GetType(level2).FullName)
                ElseIf level = 3 Then
                    MoveToScreen(GetType(level3).FullName)
                End If
            End If
        End Sub

        Private Sub CustomDestroy()
            SpriteManager.RemoveSprite(level_1)
            SpriteManager.RemoveSprite(level_2)
            SpriteManager.RemoveSprite(level_3)
            SpriteManager.RemoveSprite(cursor)
            SpriteManager.RemoveSprite(levelSelect)
            SpriteManager.RemoveScene(level1, True)
        End Sub

        Private Shared Sub CustomLoadStaticContent(ByVal contentManagerName As String)


        End Sub

    End Class
End Namespace
