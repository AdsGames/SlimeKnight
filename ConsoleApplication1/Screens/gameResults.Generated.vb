Imports System.Collections.Generic
Imports System.Text
Imports FlatRedBall.Math.Geometry
Imports FlatRedBall.AI.Pathfinding
Imports FlatRedBall.Input
Imports FlatRedBall.IO
Imports FlatRedBall.Instructions
Imports FlatRedBall.Math.Splines
Imports FlatRedBall.Utilities
Imports BitmapFont = FlatRedBall.Graphics.BitmapFont

Imports Cursor = FlatRedBall.Gui.Cursor
Imports GuiManager = FlatRedBall.Gui.GuiManager

#If XNA4 OrElse WINDOWS_8 Then
Imports Color = Microsoft.Xna.Framework.Color
#Else
Imports Color = Microsoft.Xna.Framework.Graphics.Color
#End If

#If FRB_XNA OrElse SILVERLIGHT Then
Imports Keys = Microsoft.Xna.Framework.Input.Keys
Imports Vector3 = Microsoft.Xna.Framework.Vector3
Imports Texture2D = Microsoft.Xna.Framework.Graphics.Texture2D
Imports Microsoft.Xna.Framework.Media
#End If

' Generated Usings
Imports FlatRedBall

Namespace Screens
	Public Partial Class gameResults
		Inherits Screen
		' Generated Fields
		#If DEBUG Then
		Shared HasBeenLoadedWithGlobalContentManager As Boolean = False
		#End If


		Public Sub New()
			MyBase.New("gameResults")
		End Sub

		Public Overrides Sub Initialize(addToManagers__1 As Boolean)
			' Generated Initialize
			LoadStaticContent(ContentManagerName)


			PostInitialize()
			MyBase.Initialize(addToManagers__1)
			If addToManagers__1 Then
				AddToManagers()
			End If

		End Sub

		' Generated AddToManagers
		Public Overrides Sub AddToManagers()
			MyBase.AddToManagers()
			AddToManagersBottomUp()
			CustomInitialize()
		End Sub


		Public Overrides Sub Activity(firstTimeCalled As Boolean)
			' Generated Activity

			If Not IsPaused Then
			Else
			End If
			MyBase.Activity(firstTimeCalled)
			If Not IsActivityFinished Then
				CustomActivity(firstTimeCalled)
			End If


			' After Custom Activity


		End Sub

		Public Overrides Sub Destroy()
			' Generated Destroy


			MyBase.Destroy()

			CustomDestroy()

		End Sub

		' Generated Methods
		Public Overridable Sub PostInitialize()
			Dim oldShapeManagerSuppressAdd As Boolean = FlatRedBall.Math.Geometry.ShapeManager.SuppressAddingOnVisibilityTrue
			FlatRedBall.Math.Geometry.ShapeManager.SuppressAddingOnVisibilityTrue = True
			FlatRedBall.Math.Geometry.ShapeManager.SuppressAddingOnVisibilityTrue = oldShapeManagerSuppressAdd
		End Sub
		Public Overridable Sub AddToManagersBottomUp()
		End Sub
		Public Overridable Sub ConvertToManuallyUpdated()
		End Sub
		Public Shared Sub LoadStaticContent(contentManagerName As String)
			#If DEBUG Then
			If contentManagerName = FlatRedBallServices.GlobalContentManager Then
				HasBeenLoadedWithGlobalContentManager = True
			ElseIf HasBeenLoadedWithGlobalContentManager Then
				Throw New Exception("This type has been loaded with a Global content manager, then loaded with a non-global.  This can lead to a lot of bugs")
			End If
			#End If
			CustomLoadStaticContent(contentManagerName)
		End Sub
		<System.Obsolete("Use GetFile instead")> _
		Public Shared Function GetStaticMember(memberName As String) As Object
			Return Nothing
		End Function
		Public Shared Function GetFile(memberName As String) As Object
			Return Nothing
		End Function
		Private Function GetMember(memberName As String) As Object
			Return Nothing
		End Function


	End Class
End Namespace
