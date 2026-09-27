'Allan Legemaate
'1/22/13
'Splash
'Loads the splashscreen
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
    Partial Public Class splash
        'Create scene
        Dim level1 As Scene

        'Create a variable to hold the tools object
        Dim tool As New tools

        'Setup sprites
        Dim start As Sprite
        Dim help As Sprite
        Dim quit As Sprite
        Dim slime As Sprite
        Dim story As Sprite

        Private Sub CustomInitialize()
            'load the scene
            level1 = tool.loadScene("Content\\splash.scnx", Me.ContentManagerName)
        End Sub

        Private Sub CustomActivity(ByVal firstTimeCalled As Boolean)
            'Key Pressed
            If Microsoft.Xna.Framework.Input.Keyboard.GetState().GetPressedKeys().Length > 0 Then
                CustomDestroy()
                MoveToScreen(GetType(menu).FullName)
            End If
        End Sub

        Private Sub CustomDestroy()
            SpriteManager.RemoveScene(level1, True)
        End Sub

        Private Shared Sub CustomLoadStaticContent(ByVal contentManagerName As String)

        End Sub

    End Class
End Namespace
