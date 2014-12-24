#Region "Using"
Imports System.Collections.Generic

Imports FlatRedBall
Imports FlatRedBall.Graphics

#If Not SILVERLIGHT Then
Imports FlatRedBall.Graphics.Model
#End If

Imports FlatRedBall.ManagedSpriteGroups

Imports FlatRedBall.Math
Imports FlatRedBall.Math.Geometry

Imports FlatRedBall.Gui
Imports FlatRedBall.Utilities
Imports FlatRedBall.IO

#If WINDOWS_PHONE Then
Imports System.IO.IsolatedStorage
Imports Microsoft.Phone.Shell
#End If

#End Region

Namespace Screens

	Public NotInheritable Partial Class ScreenManager
		Private Sub New()
		End Sub
		#Region "Fields"

		Private Shared mCurrentScreen As Screen

		Private Shared mSuppressStatePush As Boolean = False

		#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
		Private Shared mBackStack As New BackStack(Of System.Nullable(Of StackItem))()
		#End If

		Private Shared mWarnIfNotEmptyBetweenScreens As Boolean = True

		Private Shared mNumberOfFramesSinceLastScreenLoad As Integer = 0

		Private Shared mNextScreenLayer As Layer

		' The ScreenManager can be told to ignore certain objects which
		' we recognize will persist from screen to screen.  This should
		' NOT be used as a solution to get around the ScreenManager's check.
		Private Shared mPersistentCameras As New PositionedObjectList(Of Camera)()

		Private Shared mPersistentSpriteFrames As New PositionedObjectList(Of SpriteFrame)()

		Private Shared mPersistentTexts As New PositionedObjectList(Of Text)()

		#End Region

		#Region "Properties"

		Public Shared ReadOnly Property CurrentScreen() As Screen
			Get
				Return mCurrentScreen
			End Get
		End Property

		Public Shared ReadOnly Property NextScreenLayer() As Layer
			Get
				Return mNextScreenLayer
			End Get
		End Property

		Public Shared ReadOnly Property PersistentCameras() As PositionedObjectList(Of Camera)
			Get
				Return mPersistentCameras
			End Get
		End Property

		Public Shared ReadOnly Property PersistentSpriteFrames() As PositionedObjectList(Of SpriteFrame)
			Get
				Return mPersistentSpriteFrames
			End Get
		End Property

		Public Shared ReadOnly Property PersistentTexts() As PositionedObjectList(Of Text)
			Get
				Return mPersistentTexts
			End Get
		End Property

		Public Shared Property WarnIfNotEmptyBetweenScreens() As Boolean
			Get
				Return mWarnIfNotEmptyBetweenScreens
			End Get
			Set
				mWarnIfNotEmptyBetweenScreens = value
			End Set
		End Property

		Public Shared Property ShouldActivateScreen() As Boolean
			Get
				Return m_ShouldActivateScreen
			End Get
			Set
				m_ShouldActivateScreen = Value
			End Set
		End Property
		Private Shared m_ShouldActivateScreen As Boolean

		Public Shared Property RehydrateAction() As Action(Of String)
			Get
				Return m_RehydrateAction
			End Get
			Set
				m_RehydrateAction = Value
			End Set
		End Property
		Private Shared m_RehydrateAction As Action(Of String)

		#End Region

		#Region "Methods"

		#Region "Public Methods"

		#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
		Public Shared Sub PushStateToStack(state As Integer)
			If Not mSuppressStatePush AndAlso PlatformServices.BackStackEnabled Then
                mBackStack.MoveTo(New StackItem() With { _
                 .State = state _
                })
			End If
		End Sub

		Public Shared Sub NavigateBack()
			If Not PlatformServices.BackStackEnabled Then
				Return
			End If

			Dim stackResult As System.Nullable(Of StackItem) = Nothing

			' screens are added when moving to a new screen, while states are added as you move
			If mBackStack.Current.HasValue AndAlso Not String.IsNullOrEmpty(mBackStack.Current.Value.Screen) Then
				stackResult = mBackStack.Current.Value
				mBackStack.Back()
			Else
				stackResult = mBackStack.Back()
			End If

			If Not stackResult.HasValue Then
				' if we've gone to the beginning of the back stack, we should exit the game
				FlatRedBallServices.Game.[Exit]()
				Return
			End If

			Dim stackItem As StackItem = stackResult.Value
			If Not String.IsNullOrEmpty(stackItem.Screen) Then
				mCurrentScreen.NextScreen = stackItem.Screen
				mCurrentScreen.IsActivityFinished = True
				mCurrentScreen.IsMovingBack = True
			ElseIf stackItem.State > -1 Then
				' states are set as they are moved to, so if this is the last state, we should exit
				If mBackStack.Count = 0 Then
					FlatRedBallServices.Game.[Exit]()
					Return
				End If

				mCurrentScreen.MoveToState(stackItem.State)
			End If
		End Sub
		#End If

		#Region "XML Docs"
		''' <summary>
		''' Calls activity on the current screen and checks to see if screen
		''' activity is finished.  If activity is finished, the current Screen's
		''' NextScreen is loaded.
		''' </summary>
		#End Region
		Public Shared Sub Activity()
			If mCurrentScreen Is Nothing Then
				Return
			End If

			mCurrentScreen.Activity(False)

			mCurrentScreen.ActivityCallCount += 1

			If mCurrentScreen.IsActivityFinished Then
				GuiManager.Cursor.IgnoreNextClick = True
				Dim type As String = mCurrentScreen.NextScreen
				Dim asyncLoadedScreen As Screen = mCurrentScreen.mNextScreenToLoadAsync

				#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
				If Not mCurrentScreen.IsMovingBack AndAlso PlatformServices.BackStackEnabled Then
                    Dim item As New StackItem() With { _
                     .Screen = mCurrentScreen.[GetType]().FullName, _
                      .State = -1 _
                    }
					mBackStack.MoveTo(item, mCurrentScreen.BackStackBehavior)
				End If
				#End If

				mCurrentScreen.Destroy()

				' check to see if there is any leftover data
				CheckAndWarnIfNotEmpty()

				' Let's perform a GC here.  
				GC.Collect()
				GC.WaitForPendingFinalizers()

				If asyncLoadedScreen Is Nothing Then

					' Loads the Screen, suspends input for one frame, and
					' calls Activity on the Screen.
					' The Activity call is required for objects like SpriteGrids
					' which need to be managed internally.

					' No need to assign mCurrentScreen - this is done by the 4th argument "true"
					'mCurrentScreen = 
					LoadScreen(type, Nothing, True, True)


					mNumberOfFramesSinceLastScreenLoad = 0
				Else

					mCurrentScreen = asyncLoadedScreen

					mCurrentScreen.AddToManagers()

					mCurrentScreen.Activity(True)


					mCurrentScreen.ActivityCallCount += 1
					mNumberOfFramesSinceLastScreenLoad = 0
				End If
			Else
				mNumberOfFramesSinceLastScreenLoad += 1
			End If
		End Sub


		Public Shared Function LoadScreen(screen As String, createNewLayer As Boolean) As Screen
			If createNewLayer Then
				Return LoadScreen(screen, SpriteManager.AddLayer())
			Else
				Return LoadScreen(screen, DirectCast(Nothing, Layer))
			End If
		End Function


		Public Shared Function LoadScreen(Of T As Screen)(layerToLoadScreenOn As Layer) As T
			mNextScreenLayer = layerToLoadScreenOn

			#If XBOX360 Then
			Dim newScreen As T = DirectCast(Activator.CreateInstance(GetType(T)), T)
			#Else
			Dim newScreen As T = DirectCast(Activator.CreateInstance(GetType(T), New Object(-1) {}), T)
			#End If

			FlatRedBall.Input.InputManager.CurrentFrameInputSuspended = True

			newScreen.Initialize(True)

			#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
			If mBackStack.Current.HasValue AndAlso mBackStack.Current.Value.State > -1 AndAlso PlatformServices.BackStackEnabled Then
				newScreen.MoveToState(mBackStack.Current.Value.State)
			End If
			#End If

			newScreen.Activity(True)

			newScreen.ActivityCallCount += 1

			Return newScreen
		End Function


		Public Shared Function LoadScreen(screen As String, layerToLoadScreenOn As Layer) As Screen
			Return LoadScreen(screen, layerToLoadScreenOn, True, False)
		End Function

		Public Shared Function LoadScreen(screen As String, layerToLoadScreenOn As Layer, addToManagers As Boolean, makeCurrentScreen As Boolean) As Screen
			mNextScreenLayer = layerToLoadScreenOn

			Dim newScreen As Screen = Nothing

			Dim typeOfScreen As Type = Type.[GetType](screen)

			If typeOfScreen Is Nothing Then
				Throw New System.ArgumentException((Convert.ToString("There is no ") & screen) + " class defined in your project or linked assemblies.")
			End If

			If screen IsNot Nothing AndAlso screen <> "" Then
				#If XBOX360 Then
				newScreen = DirectCast(Activator.CreateInstance(typeOfScreen), Screen)
				#Else
					#End If
				newScreen = DirectCast(Activator.CreateInstance(typeOfScreen, New Object(-1) {}), Screen)
			End If

			If newScreen IsNot Nothing Then
				FlatRedBall.Input.InputManager.CurrentFrameInputSuspended = True

				#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
				mSuppressStatePush = mBackStack.Current.HasValue AndAlso mBackStack.Current.Value.State > -1
				#End If

				newScreen.Initialize(addToManagers)

				#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
				If mSuppressStatePush Then
					newScreen.MoveToState(mBackStack.Current.Value.State)
				End If
				#End If
				mSuppressStatePush = False

				If addToManagers Then
					' We do this so that new Screens are the CurrentScreen in Activity.
					' This is useful in custom logic.
					If makeCurrentScreen Then
						mCurrentScreen = newScreen
					End If

					newScreen.Activity(True)


					newScreen.ActivityCallCount += 1
				End If
			End If

			Return newScreen
		End Function

		Public Shared Sub Start(Of T As {Screen, New})()
			mCurrentScreen = LoadScreen(Of T)(Nothing)
		End Sub

		#Region "XML Docs"
		''' <summary>
		''' Loads a screen.  Should only be called once during initialization.
		''' </summary>
		''' <param name="screenToStartWith">Qualified name of the class to load.</param>
		#End Region
		Public Shared Sub Start(screenToStartWith As String)
			If mCurrentScreen IsNot Nothing Then
				Throw New InvalidOperationException("You can't call Start if there is already a Screen.  Did you call Start twice?")
			Else
				#If Not FRB_MDX Then
				AddHandler StateManager.Current.Activating, New Action(AddressOf OnStateActivating)
				AddHandler StateManager.Current.Deactivating, New Action(AddressOf OnStateDeactivating)
				StateManager.Current.Initialize()

				#If Not MONODROID AndAlso Not SILVERLIGHT Then
				'if the state manager overwrote the backstack from tombstone (WP7), it will have a different current, 
				'otherwise, it will be the same value as screenToStartWith.
				If mBackStack.Count > 0 Then
					System.Diagnostics.Debug.WriteLine("resuming with backstack containing " + mBackStack.Count + " items with current being " + mBackStack.Current.Value.Screen)
					screenToStartWith = mBackStack.Current.Value.Screen
					mBackStack.Back()
				End If
				#End If
				#End If

				If ShouldActivateScreen AndAlso RehydrateAction IsNot Nothing Then
                    'RehydrateAction = screenToStartWith
                    ' RehydrateAction = (screenToStartWith)
                Else
                    mCurrentScreen = LoadScreen(screenToStartWith, Nothing, True, True)

                    ShouldActivateScreen = False
                End If
            End If
        End Sub

        ''' <summary>Do all state deactivation work here.</summary>
        Private Shared Sub OnStateDeactivating()
#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
            mBackStack.MoveTo(New StackItem() With {.Screen = mCurrentScreen.[GetType]().FullName, .State = -1})
            StateManager.Current("backstack") = mBackStack
#End If
        End Sub

        ''' <summary>Do all state activation work here</summary>
        Private Shared Sub OnStateActivating()
#If Not MONODROID AndAlso Not FRB_MDX AndAlso Not SILVERLIGHT Then
            mBackStack = StateManager.Current.[Get](Of BackStack(Of System.Nullable(Of StackItem)))("backstack")

            ShouldActivateScreen = True
#End If
        End Sub


        Public Shared Shadows Function ToString() As String
            If mCurrentScreen IsNot Nothing Then
                Return mCurrentScreen.ToString()
            Else
                Return "No Current Screen"
            End If
        End Function

#End Region

#Region "Private Methods"
        Public Shared Sub CheckAndWarnIfNotEmpty()
            If WarnIfNotEmptyBetweenScreens Then
                Dim messages As New List(Of String)()
                ' the user wants to make sure that the Screens have cleaned up everything
                ' after being destroyed.  Check the data to make sure it's all empty.

                '#Region "Make sure there's only 1 non-persistent Camera left"
                If SpriteManager.Cameras.Count > 1 Then
                    Dim count As Integer = SpriteManager.Cameras.Count

                    For Each camera As Camera In mPersistentCameras
                        If SpriteManager.Cameras.Contains(camera) Then
                            count -= 1
                        End If
                    Next

                    If count > 1 Then
                        messages.Add("There are " + count + " Cameras in the SpriteManager (excluding ignored Cameras).  There should only be 1.")
                    End If
                End If
                '#End Region

                '#Region "Make sure that the Camera doesn't have any extra layers"

                If SpriteManager.Camera.Layers.Count > 1 Then
                    messages.Add("There are " + SpriteManager.Camera.Layers.Count + " Layers on the default Camera.  There should only be 1")
                End If

                '#End Region

                '#Region "Automatically updated Sprites"
                If SpriteManager.AutomaticallyUpdatedSprites.Count <> 0 Then
                    Dim spriteCount As Integer = SpriteManager.AutomaticallyUpdatedSprites.Count

                    For Each spriteFrame As SpriteFrame In mPersistentSpriteFrames
                        For Each sprite As Sprite In SpriteManager.AutomaticallyUpdatedSprites
                            If spriteFrame.IsSpriteComponentOfThis(sprite) Then
                                spriteCount -= 1
                            End If
                        Next
                    Next

                    If spriteCount <> 0 Then
                        messages.Add("There are " + Convert.ToString(spriteCount) + " AutomaticallyUpdatedSprites in the SpriteManager.")

                    End If
                End If
                '#End Region

                '#Region "Manually updated Sprites"
                If SpriteManager.ManuallyUpdatedSpriteCount <> 0 Then
                    messages.Add("There are " + SpriteManager.ManuallyUpdatedSpriteCount + " ManuallyUpdatedSprites in the SpriteManager.")
                End If
                '#End Region

                '#Region "Ordered by distance Sprites"

                If SpriteManager.OrderedSprites.Count <> 0 Then
                    Dim spriteCount As Integer = SpriteManager.OrderedSprites.Count

                    For Each spriteFrame As SpriteFrame In mPersistentSpriteFrames
                        For Each sprite As Sprite In SpriteManager.OrderedSprites
                            If spriteFrame.IsSpriteComponentOfThis(sprite) Then
                                spriteCount -= 1
                            End If
                        Next
                    Next

                    If spriteCount <> 0 Then
                        messages.Add("There are " + Convert.ToString(spriteCount) + " Ordered (Drawn) Sprites in the SpriteManager.")

                    End If
                End If

                '#End Region

                '#Region "Managed Positionedobjects"
                If SpriteManager.ManagedPositionedObjects.Count <> 0 Then
                    messages.Add("There are " + SpriteManager.ManagedPositionedObjects.Count + " Managed PositionedObjects in the SpriteManager.")
                End If

                '#End Region

                '#Region "Layers"
                If SpriteManager.LayerCount <> 0 Then
                    messages.Add("There are " + SpriteManager.LayerCount + " Layers in the SpriteManager.")
                End If

                '#End Region

                '#Region "TopLayer"

                If SpriteManager.TopLayer.Sprites.Count <> 0 Then
                    messages.Add("There are " + SpriteManager.TopLayer.Sprites.Count + " Sprites in the SpriteManager's TopLayer.")
                End If

                '#End Region

                '#Region "Particles"
                If SpriteManager.ParticleCount <> 0 Then
                    messages.Add("There are " + SpriteManager.ParticleCount + " Particle Sprites in the SpriteManager.")
                End If

                '#End Region

                '#Region "SpriteFrames"
                If SpriteManager.SpriteFrames.Count <> 0 Then
                    Dim spriteFrameCount As Integer = SpriteManager.SpriteFrames.Count

                    For Each spriteFrame As SpriteFrame In mPersistentSpriteFrames
                        If SpriteManager.SpriteFrames.Contains(spriteFrame) Then
                            spriteFrameCount -= 1
                        End If
                    Next

                    If spriteFrameCount <> 0 Then
                        messages.Add("There are " + spriteFrameCount + " SpriteFrames in the SpriteManager.")

                    End If
                End If
                '#End Region

                '#Region "Text objects"
                If TextManager.AutomaticallyUpdatedTexts.Count <> 0 Then
                    Dim textCount As Integer = TextManager.AutomaticallyUpdatedTexts.Count

                    For Each text As Text In mPersistentTexts
                        If TextManager.AutomaticallyUpdatedTexts.Contains(text) Then
                            textCount -= 1
                        End If
                    Next

                    If textCount <> 0 Then
                        messages.Add("There are " + Convert.ToString(textCount) + "automatically updated Texts in the TextManager.")
                    End If
                End If
                '#End Region

                '#Region "Managed Shapes"
                If ShapeManager.AutomaticallyUpdatedShapes.Count <> 0 Then
                    messages.Add("There are " + ShapeManager.AutomaticallyUpdatedShapes.Count + " Automatically Updated Shapes in the ShapeManager.")
                End If
                '#End Region

                '#Region "Visible Circles"
                If ShapeManager.VisibleCircles.Count <> 0 Then
                    messages.Add("There are " + ShapeManager.VisibleCircles.Count + " visible Circles in the ShapeManager.")
                End If
                '#End Region

                '#Region "Visible Rectangles"

                If ShapeManager.VisibleRectangles.Count <> 0 Then
                    messages.Add("There are " + ShapeManager.VisibleRectangles.Count + " visible AxisAlignedRectangles in the VisibleRectangles.")
                End If

                '#End Region

                '#Region "Visible Polygons"

                If ShapeManager.VisiblePolygons.Count <> 0 Then
                    messages.Add("There are " + ShapeManager.VisiblePolygons.Count + " visible Polygons in the ShapeManager.")
                End If
                '#End Region

                '#Region "Visible Lines"

                If ShapeManager.VisibleLines.Count <> 0 Then
                    messages.Add("There are " + ShapeManager.VisibleLines.Count + " visible Lines in the ShapeManager.")
                End If
                '#End Region

                '#Region "Automatically Updated Positioned Models"
#If Not SILVERLIGHT AndAlso Not MONODROID AndAlso Not WINDOWS_8 Then
                If ModelManager.AutomaticallyUpdatedModels.Count <> 0 Then
                    messages.Add("There are " + ModelManager.AutomaticallyUpdatedModels.Count + " managed PositionedModels in the ModelManager.")
                End If
#End If
                '#End Region

                If messages.Count <> 0 Then
                    Dim errorString As String = "The Screen that was just unloaded did not clean up after itself:"
                    For Each s As String In messages
                        errorString += Convert.ToString(vbLf) & s
                    Next

                    Throw New System.Exception(errorString)
                End If
            End If
        End Sub
#End Region

#End Region
    End Class
End Namespace

