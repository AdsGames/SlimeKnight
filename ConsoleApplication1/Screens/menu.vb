'Allan Legemaate
'1/22/13
'Menu
'Allows navigation through program
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
    Partial Public Class menu
        'Create scene
        Dim level1 As Scene

        'Create a variable to hold the tools object
        Dim tool As New tools

        'Setup sprites
        Dim start As Sprite
        Dim help As Sprite
        Dim quit As Sprite
        Dim story As Sprite
        Dim cursor As Sprite

        'If button selected
        Dim buttonSelected As Integer = -1

        'Setup sound effects
        Dim click As SoundEffect
        Dim music As Song

        Private Sub CustomInitialize()
            'load the scene
            level1 = tool.loadScene("Content\\menu.scnx", Me.ContentManagerName)

            'load sprites
            start = tool.loadSprite("start")
            help = tool.loadSprite("help")
            quit = tool.loadSprite("quit")
            story = tool.loadSprite("story")
            cursor = SpriteManager.AddSprite("Content\\images\\cursor.PNG")
            cursor.Z = 10
            cursor.ScaleX = 1
            cursor.ScaleY = 1

            tool.mapCollisionRectangle(cursor)
            tool.mapCollisionRectangle(start)
            tool.mapCollisionRectangle(help)
            tool.mapCollisionRectangle(quit)

            'Set selected button
            start.Texture = tool.changeTexture("Content\\images\\buttons\\start_hover.png")

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

            If story.Visible = False Then
                'Select Menu Componants
                If cursor.CollideAgainst(start) Then
                    start.Texture = tool.changeTexture("Content\\images\\buttons\\start_hover.png")
                    help.Texture = tool.changeTexture("Content\\images\\buttons\\help.png")
                    quit.Texture = tool.changeTexture("Content\\images\\buttons\\quit.png")
                    If buttonSelected <> 0 Then
                        click.Play()
                    End If
                    buttonSelected = 0
                ElseIf cursor.CollideAgainst(help) Then
                    help.Texture = tool.changeTexture("Content\\images\\buttons\\help_hover.png")
                    start.Texture = tool.changeTexture("Content\\images\\buttons\\start.png")
                    quit.Texture = tool.changeTexture("Content\\images\\buttons\\quit.png")
                    If buttonSelected <> 1 Then
                        click.Play()
                    End If
                    buttonSelected = 1
                ElseIf cursor.CollideAgainst(quit) Then
                    quit.Texture = tool.changeTexture("Content\\images\\buttons\\quit_hover.png")
                    help.Texture = tool.changeTexture("Content\\images\\buttons\\help.png")
                    start.Texture = tool.changeTexture("Content\\images\\buttons\\start.png")
                    If buttonSelected <> 2 Then
                        click.Play()
                    End If
                    buttonSelected = 2
                Else
                    quit.Texture = tool.changeTexture("Content\\images\\buttons\\quit.png")
                    start.Texture = tool.changeTexture("Content\\images\\buttons\\start.png")
                    help.Texture = tool.changeTexture("Content\\images\\buttons\\help.png")
                    buttonSelected = False
                End If

                If InputManager.Mouse.ButtonPushed(FlatRedBall.Input.Mouse.MouseButtons.LeftButton) = True Then
                    If cursor.CollideAgainst(start) Then
                        CustomDestroy()
                    ElseIf cursor.CollideAgainst(help) Then
                        story.Texture = tool.changeTexture("Content\\images\\help.png")
                        story.Visible = True
                    ElseIf cursor.CollideAgainst(quit) Then
                        tool.exitGame()
                    End If
                End If

            ElseIf InputManager.Mouse.ButtonPushed(FlatRedBall.Input.Mouse.MouseButtons.LeftButton) = True And story.Visible = True Then
                story.Visible = False
            End If
        End Sub

        Private Sub CustomDestroy()
            SpriteManager.RemoveSprite(start)
            SpriteManager.RemoveSprite(help)
            SpriteManager.RemoveSprite(quit)
            SpriteManager.RemoveSprite(story)
            SpriteManager.RemoveSprite(cursor)
            SpriteManager.RemoveScene(level1, True)
            MoveToScreen(GetType(levelSelect).FullName)
        End Sub

        Private Shared Sub CustomLoadStaticContent(ByVal contentManagerName As String)


        End Sub

    End Class
End Namespace
