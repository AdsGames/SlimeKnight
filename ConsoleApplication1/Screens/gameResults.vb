'Allan Legemaate
'1/22/13
'Game results
'Final game score
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
    Partial Public Class gameResults
        'Create the level
        Dim level1 As Scene

        'Create a variable to hold the tools object
        Dim tool As New tools

        'Text
        Dim kills As Text
        Dim damage As Text
        Dim power As Text
        Dim energy As Text

        'Music
        Dim music As Song

        Private Sub CustomInitialize()
            'load the scene
            level1 = tool.loadScene("Content\\levelEnd.scnx", Me.ContentManagerName)

            'Text
            kills = tool.loadText("kills")
            damage = tool.loadText("damage")
            power = tool.loadText("power")
            energy = tool.loadText("energy")

            kills.DisplayText = statKills
            damage.DisplayText = statDamage / 10
            power.DisplayText = statPower / 10
            energy.DisplayText = statEnergy / 10

            'Camera to 0
            SpriteManager.Camera.X = 0
            SpriteManager.Camera.Y = 0

            'Load music
            music = FlatRedBallServices.Load(Of Song)("Content\\sounds\\postCarnage", ContentManagerName)
            Microsoft.Xna.Framework.Media.MediaPlayer.Play(music)
            MediaPlayer.IsRepeating = True
        End Sub

        Private Sub CustomActivity(ByVal firstTimeCalled As Boolean)
            If tool.isKeyPressed(Keys.Enter) Then
                CustomDestroy()
                MoveToScreen(GetType(levelSelect).FullName)
            End If

        End Sub

        Private Sub CustomDestroy()
            TextManager.RemoveText(kills)
            TextManager.RemoveText(damage)
            TextManager.RemoveText(power)
            TextManager.RemoveText(energy)
            SpriteManager.RemoveScene(level1, True)
        End Sub

        Private Shared Sub CustomLoadStaticContent(ByVal contentManagerName As String)


        End Sub
    End Class
End Namespace
