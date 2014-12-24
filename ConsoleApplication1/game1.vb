Imports Microsoft.Xna.Framework
Imports Microsoft.Xna.Framework.Graphics
Imports Microsoft.Xna.Framework.Input

Imports FlatRedBall
Imports FlatRedBall.Math
Imports FlatRedBall.Math.Geometry
Imports FlatRedBall.Graphics
Imports FlatRedBall.Graphics.Particle
Imports FlatRedBall.Input

Public Class game1
    Inherits Game

    Private graphics As GraphicsDeviceManager

    Public Sub New()
        graphics = New GraphicsDeviceManager(Me)

    End Sub

    Protected Overrides Sub Initialize()
        'Set up my graphics
        Dim grOp As New GraphicsOptions
        grOp.SuspendDeviceReset()
        grOp.SetResolution(1280, 960)
        grOp.ResumeDeviceReset()
        FlatRedBallServices.InitializeFlatRedBall(Me, Me.graphics, grOp)
        FlatRedBallServices.GraphicsOptions.TextureFilter = TextureFilter.Point

        'Set up the camera
        'SpriteManager.Camera.Orthogonal = False
        'SpriteManager.Camera.OrthogonalWidth = 20
        'SpriteManager.Camera.OrthogonalHeight = 15


        'the following line of code sets up the first screen to be shown
        Screens.ScreenManager.Start(GetType(ConsoleApplication1.Screens.splash).FullName)

        MyBase.Initialize()

    End Sub

    Protected Overrides Sub Update(ByVal gameTime As Microsoft.Xna.Framework.GameTime)
        FlatRedBallServices.Update(gameTime)

        Screens.ScreenManager.Activity()

        MyBase.Update(gameTime)
    End Sub

    Protected Overrides Sub Draw(ByVal gameTime As Microsoft.Xna.Framework.GameTime)
        FlatRedBallServices.Draw()
        MyBase.Draw(gameTime)
    End Sub
End Class