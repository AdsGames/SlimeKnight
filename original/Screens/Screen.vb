#Region "Using"

Imports System.Collections.Generic
Imports System.Text

Imports FlatRedBall
Imports FlatRedBall.Math
Imports FlatRedBall.Math.Geometry
Imports FlatRedBall.Gui
Imports FlatRedBall.Instructions
#If Not SILVERLIGHT Then

Imports FlatRedBall.Graphics.Model
#End If

Imports FlatRedBall.ManagedSpriteGroups
Imports FlatRedBall.Graphics
Imports FlatRedBall.Utilities



Imports PolygonSaveList = FlatRedBall.Content.Polygon.PolygonSaveList
Imports System.Threading
Imports FlatRedBall.Input
Imports FlatRedBall.IO

#End Region

' Test

Namespace Screens
	Public Enum AsyncLoadingState
		NotStarted
		LoadingScreen
		Done
	End Enum

	Public Class Screen
		#Region "Fields"

		Protected IsPaused As Boolean = False

		Protected mTimeScreenWasCreated As Double
		Protected mAccumulatedPausedTime As Double = 0

		Protected mCamera As Camera
		Protected mLayer As Layer



		Public Property ShouldRemoveLayer() As Boolean
			Get
				Return m_ShouldRemoveLayer
			End Get
			Set
				m_ShouldRemoveLayer = Value
			End Set
		End Property
		Private m_ShouldRemoveLayer As Boolean

		Public ReadOnly Property PauseAdjustedCurrentTime() As Double
			Get
				Return TimeManager.CurrentTime - mAccumulatedPausedTime
			End Get
		End Property


		Protected mPopups As New List(Of Screen)()

		Private mContentManagerName As String


		' The following are objects which belong to the screen.
		' These are removed by the Screen when it is Destroyed
		Protected mSprites As New SpriteList()
		Protected mSpriteGrids As New List(Of SpriteGrid)()
		Protected mSpriteFrames As New PositionedObjectList(Of SpriteFrame)()

		Protected mDrawableBatches As New List(Of IDrawableBatch)()
		' End of objects which belong to the Screen.

		' These variables control the flow from one Screen to the next.


		Protected mLastLoadedScene As Scene
		Private mIsActivityFinished As Boolean
		Private mNextScreen As String

		Private mManageSpriteGrids As Boolean

		Friend mNextScreenToLoadAsync As Screen

		#If Not FRB_MDX Then
		Private ActivatingAction As Action
		Private DeactivatingAction As Action
		#End If

		#End Region

		#Region "Properties"



		Public Property ActivityCallCount() As Integer
			Get
				Return m_ActivityCallCount
			End Get
			Set
				m_ActivityCallCount = Value
			End Set
		End Property
		Private m_ActivityCallCount As Integer

		Public ReadOnly Property ContentManagerName() As String
			Get
				Return mContentManagerName
			End Get
		End Property

		#Region "XML Docs"
		''' <summary>
		''' Gets and sets whether the activity is finished for a particular screen.
		''' </summary>
		''' <remarks>
		''' If activity is finished, then the ScreenManager or parent
		''' screen (if the screen is a popup) knows to destroy the screen
		''' and loads the NextScreen class.</remarks>
		#End Region
		Public Property IsActivityFinished() As Boolean
			Get
				Return mIsActivityFinished
			End Get
			Set
				mIsActivityFinished = value
			End Set
		End Property



		Public Property AsyncLoadingState() As AsyncLoadingState
			Get
				Return m_AsyncLoadingState
			End Get
			Private Set
				m_AsyncLoadingState = Value
			End Set
		End Property
		Private m_AsyncLoadingState As AsyncLoadingState


		Public Property Layer() As Layer
			Get
				Return mLayer
			End Get
			Set
				mLayer = value
			End Set
		End Property


		Public Property ManageSpriteGrids() As Boolean
			Get
				Return mManageSpriteGrids
			End Get
			Set
				mManageSpriteGrids = value
			End Set
		End Property

		#Region "XML Docs"
		''' <summary>
		''' The fully qualified path of the Screen-inheriting class that this screen is 
		''' to link to.
		''' </summary>
		''' <remarks>
		''' This property is read by the ScreenManager when IsActivityFinished is
		''' set to true.  Therefore, this must always be set to some value before
		''' or in the same frame as when IsActivityFinished is set to true.
		''' </remarks>
		#End Region
		Public Property NextScreen() As String
			Get
				Return mNextScreen
			End Get
			Set
				mNextScreen = value
			End Set
		End Property

		Public Property IsMovingBack() As Boolean
			Get
				Return m_IsMovingBack
			End Get
			Set
				m_IsMovingBack = Value
			End Set
		End Property
		Private m_IsMovingBack As Boolean

		#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
		Public BackStackBehavior As BackStackBehavior = BackStackBehavior.Move
		#End If


		Protected Property UnloadsContentManagerWhenDestroyed() As Boolean
			Get
				Return m_UnloadsContentManagerWhenDestroyed
			End Get
			Set
				m_UnloadsContentManagerWhenDestroyed = Value
			End Set
		End Property
		Private m_UnloadsContentManagerWhenDestroyed As Boolean

		#End Region

		#Region "Methods"

		#Region "Constructor"

		Public Sub New(contentManagerName As String)
			ShouldRemoveLayer = True
			UnloadsContentManagerWhenDestroyed = True
			mContentManagerName = contentManagerName
			mManageSpriteGrids = True
			IsActivityFinished = False

			mLayer = ScreenManager.NextScreenLayer

			#If Not FRB_MDX Then
			ActivatingAction = New Action(AddressOf Activating)
			DeactivatingAction = New Action(AddressOf OnDeactivating)

			AddHandler StateManager.Current.Activating, ActivatingAction
			AddHandler StateManager.Current.Deactivating, DeactivatingAction

			If ScreenManager.ShouldActivateScreen Then
				Activating()
				#End If
			End If
		End Sub

		#End Region

		#Region "Public Methods"
		#If Not FRB_MDX Then
		Public Overridable Sub Activating()
			Me.PreActivate()
			' for generated code to override, to reload the statestack
			Me.OnActivate(PlatformServices.State)
			' for user created code
		End Sub


		Private Sub OnDeactivating()
			Me.PreDeactivate()
			' for generated code to override, to save the statestack
			Me.OnDeactivate(PlatformServices.State)
			' for user generated code;
		End Sub
		#End If
		#Region "Activation Methods"
		#If Not FRB_MDX Then
		Protected Overridable Sub OnActivate(state As StateManager)
		End Sub

		Protected Overridable Sub PreActivate()
		End Sub

		Protected Overridable Sub OnDeactivate(state As StateManager)
		End Sub

		Protected Overridable Sub PreDeactivate()
		End Sub
		#End If
		#End Region

		Public Overridable Sub Activity(firstTimeCalled As Boolean)
			If IsPaused Then
				mAccumulatedPausedTime += TimeManager.SecondDifference
			End If

			If mManageSpriteGrids Then
				For i As Integer = 0 To mSpriteGrids.Count - 1
					Dim sg As SpriteGrid = mSpriteGrids(i)
					sg.Manage()
				Next
			End If

			For i As Integer = mPopups.Count - 1 To -1 + 1 Step -1
				Dim popup As Screen = mPopups(i)

				popup.Activity(False)
				popup.ActivityCallCount += 1

				If popup.IsActivityFinished Then
					Dim nextPopup As String = popup.NextScreen

					popup.Destroy()
					mPopups.RemoveAt(i)

					If nextPopup <> "" AndAlso nextPopup IsNot Nothing Then
						LoadPopup(nextPopup, False)
					End If
				End If
			Next

			#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
			' This needs to happen after popup activity
			' in case the Screen creates a popup - we don't
			' want 2 activity calls for one frame.  We also want
			' to make sure that popups have the opportunity to handle
			' back calls so that the base doesn't get it.
			If PlatformServices.BackStackEnabled AndAlso InputManager.BackPressed AndAlso Not firstTimeCalled Then
				Me.HandleBackNavigation()
			End If
			#End If
		End Sub

		Private asyncScreenTypeToLoad As Type = Nothing


		Public Sub StartAsyncLoad(screenType As String)
			If AsyncLoadingState = Screens.AsyncLoadingState.LoadingScreen Then
				#If DEBUG Then
					#End If
                Throw New InvalidOperationException("This Screen is already loading a Screen of type " & asyncScreenTypeToLoad.FullName & ".  This is a DEBUG-only exception")
			ElseIf AsyncLoadingState = Screens.AsyncLoadingState.Done Then
				#If DEBUG Then
					#End If
                Throw New InvalidOperationException("This Screen has already loaded a Screen of type " & asyncScreenTypeToLoad.FullName & ".  This is a DEBUG-only exception")
			Else

				asyncScreenTypeToLoad = Type.[GetType](screenType)

				If asyncScreenTypeToLoad Is Nothing Then
					Throw New Exception(Convert.ToString("Could not find the type ") & screenType)
				End If
				AsyncLoadingState = AsyncLoadingState.LoadingScreen

				#If WINDOWS_8 Then
				System.Threading.Tasks.Task.Run(DirectCast(AddressOf PerformAsyncLoad, Action))
				#Else
				Dim threadStart As New ThreadStart(AddressOf PerformAsyncLoad)
				Dim thread As New Thread(threadStart)
					#End If
				thread.Start()
			End If
		End Sub

		Private Sub PerformAsyncLoad()
			#If XBOX360 Then

			' We can not use threads 0 or 2  
			Thread.CurrentThread.SetProcessorAffinity(4)
			mNextScreenToLoadAsync = DirectCast(Activator.CreateInstance(asyncScreenTypeToLoad), Screen)
			#Else
			mNextScreenToLoadAsync = DirectCast(Activator.CreateInstance(asyncScreenTypeToLoad, New Object(-1) {}), Screen)
			#End If
			' Don't add it to the manager!
			mNextScreenToLoadAsync.Initialize(False)

			AsyncLoadingState = AsyncLoadingState.Done
		End Sub

		Public Overridable Sub Initialize(addToManagers As Boolean)

		End Sub


		Public Overridable Sub AddToManagers()
			' We want to start the timer when we actually add to managers - this is when the activity for the Screen starts
			mAccumulatedPausedTime = TimeManager.CurrentTime
			mTimeScreenWasCreated = TimeManager.CurrentTime
		End Sub


		Public Overridable Sub Destroy()
			#If Not FRB_MDX Then
			RemoveHandler StateManager.Current.Activating, ActivatingAction
			RemoveHandler StateManager.Current.Deactivating, DeactivatingAction
			#End If
			If mLastLoadedScene IsNot Nothing Then
				mLastLoadedScene.Clear()
			End If


			FlatRedBall.Debugging.Debugger.DestroyText()

			' All of the popups should be destroyed as well
			For Each s As Screen In mPopups
				s.Destroy()
			Next

			SpriteManager.RemoveSpriteList(Of Sprite)(mSprites)

			' It's common for users to forget to add Particle Sprites
			' to the mSprites SpriteList.  This will either create leftover
			' particles when the next screen loads or will throw an assert when
			' the ScreenManager checks if there are any leftover Sprites.  To make
			' things easier we'll just clear the Particle Sprites here.
			Dim isPopup As Boolean = Me IsNot ScreenManager.CurrentScreen
			If Not isPopup Then
				SpriteManager.RemoveAllParticleSprites()
			End If

			' Destory all SpriteGrids that belong to this Screen
			For Each sg As SpriteGrid In mSpriteGrids
				sg.Destroy()
			Next


			' Destroy all SpriteFrames that belong to this Screen
			While mSpriteFrames.Count <> 0
				SpriteManager.RemoveSpriteFrame(mSpriteFrames(0))
			End While

			If UnloadsContentManagerWhenDestroyed AndAlso mContentManagerName <> FlatRedBallServices.GlobalContentManager Then
				FlatRedBallServices.Unload(mContentManagerName)
				FlatRedBallServices.Clean()
			End If

			If ShouldRemoveLayer AndAlso mLayer IsNot Nothing Then
				SpriteManager.RemoveLayer(mLayer)
			End If
			If IsPaused Then
				UnpauseThisScreen()
			End If

			GuiManager.Cursor.IgnoreNextClick = True
		End Sub

		Protected Overridable Sub PauseThisScreen()
			'base.PauseThisScreen();

			Me.IsPaused = True
			InstructionManager.PauseEngine()

		End Sub

		Protected Overridable Sub UnpauseThisScreen()
			InstructionManager.UnpauseEngine()
			Me.IsPaused = False
		End Sub

		Public Function PauseAdjustedSecondsSince(time As Double) As Double
			Return PauseAdjustedCurrentTime - time
		End Function

		#Region "XML Docs"
		''' <summary>Tells the screen that we are done and wish to move to the
		''' supplied screen</summary>
		''' <param>Fully Qualified Type of the screen to move to</param>
		#End Region
		Public Sub MoveToScreen(screenClass As String)
			IsActivityFinished = True
			NextScreen = screenClass
		End Sub

		#End Region

		#Region "Protected Methods"

		Public Function LoadPopup(Of T As Screen)(layerToLoadPopupOn As Layer) As T
			Dim loadedScreen As T = ScreenManager.LoadScreen(Of T)(layerToLoadPopupOn)
			mPopups.Add(loadedScreen)
			Return loadedScreen
		End Function

		Public Function LoadPopup(popupToLoad As String, layerToLoadPopupOn As Layer) As Screen
			Return LoadPopup(popupToLoad, layerToLoadPopupOn, True)
		End Function

		Public Function LoadPopup(popupToLoad As String, layerToLoadPopupOn As Layer, addToManagers As Boolean) As Screen
			Dim loadedScreen As Screen = ScreenManager.LoadScreen(popupToLoad, layerToLoadPopupOn, addToManagers, False)
			mPopups.Add(loadedScreen)
			Return loadedScreen
		End Function

		Public Function LoadPopup(popupToLoad As String, useNewLayer As Boolean) As Screen
			Dim loadedScreen As Screen = ScreenManager.LoadScreen(popupToLoad, useNewLayer)
			mPopups.Add(loadedScreen)
			Return loadedScreen
		End Function

		''' <param name="state">This should be a valid enum value of the concrete screen type.</param>
		Public Overridable Sub MoveToState(state As Integer)
			' no-op
		End Sub

		''' <summary>Default implementation tells the screen manager to finish this screen's activity and navigate
		''' to the previous screen on the backstack.</summary>
		''' <remarks>Override this method if you want to have custom behavior when the back button is pressed.</remarks>
		Protected Overridable Sub HandleBackNavigation()
			' This is to prevent popups from unexpectedly going back
			If ScreenManager.CurrentScreen Is Me Then
				#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
					#End If
				ScreenManager.NavigateBack()
			End If
		End Sub

		#End Region

		#End Region
	End Class
End Namespace
