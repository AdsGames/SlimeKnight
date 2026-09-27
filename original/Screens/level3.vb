'Allan Legemaate
'1/22/13
'Level 3
'Third level
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

Imports FlatRedBall.Audio.AudioManager

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
Imports FlatRedBall.Math

#If FRB_XNA OrElse SILVERLIGHT Then
Imports Keys = Microsoft.Xna.Framework.Input.Keys
Imports Vector3 = Microsoft.Xna.Framework.Vector3
Imports Texture2D = Microsoft.Xna.Framework.Graphics.Texture2D
#End If

Namespace Screens
    Partial Public Class level3

        'Create a variable to hold the scene
        Dim level1 As Scene

        'Create a variable to hold the tools object
        Dim tool As New tools

        'Create character 
        Dim character As Sprite
        Dim characterBounds As Rectangle

        'HUD text, health and energy bars
        Dim hud As Sprite
        Dim healthDraw As Sprite
        Dim energyDraw As Sprite
        Dim powerDraw As Sprite
        Dim healthText As Text
        Dim powerText As Text
        Dim energyText As Text

        'The changable character attributes
        Dim speed As Single
        Dim extraSpeed As Single
        Dim energy As Integer
        Dim energyRegen As Integer
        Dim power As Integer
        Dim invincible As Boolean
        Dim health As Integer
        Dim slash As Boolean
        Dim superSlash As Boolean
        Dim direction As Integer
        Dim beingHurt As Boolean

        'Setup camera
        Dim camera_a = SpriteManager.Camera
        Dim camera_hud = SpriteManager.Camera

        'Screen bounds
        Dim leftEdge As Single = -50
        Dim rightEdge As Single = 50
        Dim bottomEdge As Single = -40
        Dim topEdge As Single = 40

        'Create Enemies
        Dim spawnSpeed As Integer = 220
        Dim enemies As SpriteList
        Dim blobAnimation As AnimationChainList
        Dim blobAnimation2 As AnimationChainList
        Dim blobAnimation3 As AnimationChainList

        'Create Ruins
        Dim frequencyRuins As Integer = 20
        Dim ruins As SpriteList

        'Create towers
        Dim frequencyTowers As Integer = 12
        Dim towers As SpriteList

        'Powerups
        Dim powerups As SpriteList
        Dim powerupsType As List(Of Integer)
        Dim superReady As Sprite

        'Powergem
        Dim rubbish As SpriteList

        'Spawn counter
        Dim spawnCounter As Integer

        'Sounds
        Dim splat As SoundEffect
        Dim walk As SoundEffect
        Dim slime As SoundEffect
        Dim gateOpen As SoundEffect
        Dim gateClose As SoundEffect
        Dim sword As SoundEffect
        Dim wind As SoundEffect
        Dim towerDestroy As SoundEffect
        Dim hurt As SoundEffect
        Dim drop As SoundEffect

        'BG Music
        Dim music As Song

        'Boss
        Dim boss As Sprite
        Dim bossAnimation As AnimationChainList
        Dim bossHealth As Integer
        Dim won As Boolean = False

        'Reduce lag
        Dim frameSkip As Integer

        Private Sub CustomInitialize()

            'load the scene
            level1 = tool.loadScene("Content\\level3.scnx", Me.ContentManagerName)

            'Set players direction
            direction = 0
            health = 1000
            energy = 1000
            power = 0
            energyRegen = 1

            'load the sprites
            character = tool.loadSprite("character")
            healthDraw = level1.Sprites.FindByName("health")
            energyDraw = level1.Sprites.FindByName("energy")
            powerDraw = level1.Sprites.FindByName("power")
            hud = level1.Sprites.FindByName("hud")
            superReady = level1.Sprites.FindByName("superReady")
            superReady.Visible = False

            'Sprite list
            enemies = New SpriteList
            ruins = New SpriteList
            towers = New SpriteList
            powerups = New SpriteList
            rubbish = New SpriteList
            powerupsType = New List(Of Integer)

            'Setup enemy animantions
            blobAnimation = FlatRedBallServices.Load(Of AnimationChainList)("Content\\images\\blob.achx")
            blobAnimation2 = FlatRedBallServices.Load(Of AnimationChainList)("Content\\images\\blob2.achx")
            blobAnimation3 = FlatRedBallServices.Load(Of AnimationChainList)("Content\\images\\blob3.achx")
            bossAnimation = FlatRedBallServices.Load(Of AnimationChainList)("Content\\images\\boss.achx")

            'load texts
            healthText = tool.loadText("healthText")
            energyText = tool.loadText("energyText")
            powerText = tool.loadText("powerText")

            'Set up collision rectangles
            tool.mapCollisionRectangle(character)

            'Cameras
            camera_a = New Camera(ContentManagerName)
            camera_hud = New Camera(ContentManagerName)

            SpriteManager.Cameras.Add(camera_hud)
            SpriteManager.Cameras.Add(camera_a)
            SpriteManager.Cameras(1).DrawsWorld = False
            SpriteManager.Cameras(0).Z = 40
            SpriteManager.Cameras(2).Z = 240
            SpriteManager.Cameras(0).SetSplitScreenViewport(Camera.SplitScreenViewport.FullScreen)
            SpriteManager.Cameras(2).SetSplitScreenViewport(Camera.SplitScreenViewport.TopRight)

            'Hud bar
            SpriteManager.AddToLayer(hud, SpriteManager.Cameras(1).Layer)
            SpriteManager.AddToLayer(healthDraw, SpriteManager.Cameras(1).Layer)
            SpriteManager.AddToLayer(energyDraw, SpriteManager.Cameras(1).Layer)
            SpriteManager.AddToLayer(powerDraw, SpriteManager.Cameras(1).Layer)
            SpriteManager.AddToLayer(superReady, SpriteManager.Cameras(1).Layer)
            TextManager.AddToLayer(healthText, SpriteManager.Cameras(1).Layer)
            TextManager.AddToLayer(powerText, SpriteManager.Cameras(1).Layer)
            TextManager.AddToLayer(energyText, SpriteManager.Cameras(1).Layer)

            'Sounds
            splat = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\splat")
            walk = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\grass")
            slime = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\slime1")
            gateOpen = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\gateOpen")
            gateClose = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\gateClose")
            sword = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\sword")
            hurt = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\hurt")
            drop = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\drop")
            towerDestroy = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\towerDestroy")
            wind = FlatRedBallServices.Load(Of SoundEffect)("Content\\sounds\\wind")

            'Setup ruins
            For counter As Integer = 0 To frequencyRuins
                Dim ruin As Sprite
                ruin = SpriteManager.AddSprite("Content\\images\\plywood.png")
                ruin.X = tool.random(leftEdge, rightEdge)
                ruin.Y = tool.random(bottomEdge, topEdge)
                tool.mapCollisionRectangle(ruin)
                ruins.Add(ruin)
            Next (counter)

            'Setup towers
            For counter As Integer = 0 To frequencyTowers
                Dim tower As Sprite
                tower = SpriteManager.AddSprite("Content\\images\\tower.png")
                tower.X = tool.random(leftEdge, rightEdge)
                tower.Y = tool.random(bottomEdge, topEdge)
                tower.ScaleX = 4
                tower.ScaleY = 5
                tool.mapCollisionRectangle(tower)
                towers.Add(tower)
            Next (counter)

            'Control overlap collisions tower to tower
            For counter As Integer = 0 To towers.Count
                For counter2 As Integer = 0 To towers.Count
                    If counter < towers.Count And counter2 < towers.Count Then
                        towers(counter).CollideAgainstMove(towers(counter2), 1, 1)
                    End If
                Next counter2
            Next counter

            'Control overlap collisions tower to ruin
            For counter As Integer = 0 To towers.Count
                For counter2 As Integer = 0 To ruins.Count
                    If counter < towers.Count And counter2 < ruins.Count Then
                        towers(counter).CollideAgainstMove(ruins(counter2), 1, 1)
                    End If
                Next counter2
            Next counter

            'Control overlap collisions ruin to 
            For counter As Integer = 0 To ruins.Count
                For counter2 As Integer = 0 To ruins.Count
                    If counter < ruins.Count And counter2 < ruins.Count Then
                        ruins(counter).CollideAgainstMove(ruins(counter2), 1, 1)
                    End If
                Next counter2
            Next counter

            'Load music
            music = FlatRedBallServices.Load(Of Song)("Content\\sounds\\level3", ContentManagerName)
            Microsoft.Xna.Framework.Media.MediaPlayer.Play(music)
            MediaPlayer.IsRepeating = True

            'Reset Kills
            statDamage = 0
            statKills = 0
            statPower = 0
            statEnergy = 0
        End Sub

        Private Sub CustomActivity(ByVal firstTimeCalled As Boolean)

            'Add one to frameskip (reduce lag)
            frameSkip += 1

            ''Hud
            'Hud activity
            healthDraw.ScaleX = health / 200
            energyDraw.ScaleX = energy / 200
            powerDraw.ScaleX = power / 200

            healthText.DisplayText = health / 10 & "/100"
            energyText.DisplayText = energy / 10 & "/100"
            powerText.DisplayText = power / 10 & "/100"

            'Prevents stat bar overflows
            If energy > 1000 Then
                energy = 1000
            ElseIf health > 1000 Then
                health = 1000
            ElseIf power > 1000 Then
                power = 1000
            End If


            ''Collisions
            'Control enemies
            For counter As Integer = 0 To enemies.Count
                If counter < enemies.Count Then
                    If enemies(counter).X < character.X Then
                        enemies(counter).X += 0.02
                    End If
                    If enemies(counter).X > character.X Then
                        enemies(counter).X -= 0.02
                    End If
                    If enemies(counter).Y < character.Y Then
                        enemies(counter).Y += 0.02
                    End If
                    If enemies(counter).Y > character.Y Then
                        enemies(counter).Y -= 0.02
                    End If
                    If enemies(counter).CollideAgainstMove(character, 1, 0) Then
                        If slash = True Then
                            If enemies(counter).ScaleX = 1 Then
                                Dim drop As Sprite
                                drop = SpriteManager.AddSprite("Content\\images\\slimeDeath.png")
                                drop.X = enemies(counter).X
                                drop.Y = enemies(counter).Y
                                drop.ScaleX = enemies(counter).ScaleX
                                drop.ScaleY = enemies(counter).ScaleY
                                rubbish.Add(drop)
                                SpriteManager.RemoveSprite(enemies(counter))
                                splat.Play(1, 1, 0.5)
                                power += 40
                                statPower += 40
                                statKills += 1
                            Else
                                bossHealth -= 1
                                power += 1
                                If tool.random(0, 2) = 1 Then
                                    Dim enemy As Sprite
                                    Dim type As Integer
                                    type = tool.random(0, 3)
                                    If type = 0 Then
                                        enemy = SpriteManager.AddSprite(blobAnimation)
                                    ElseIf type = 1 Then
                                        enemy = SpriteManager.AddSprite(blobAnimation2)
                                    ElseIf type = 2 Then
                                        enemy = SpriteManager.AddSprite(blobAnimation3)
                                    Else
                                        enemy = SpriteManager.AddSprite(blobAnimation)
                                    End If
                                    enemy.X = boss.X
                                    enemy.Y = boss.Y
                                    enemy.Z = 0.1
                                    tool.mapCollisionRectangle(enemy)
                                    enemies.Add(enemy)
                                    slime.Play()
                                End If
                            End If
                        Else
                            If health > 4 Then
                                If tool.random(0, 40) = 1 Then
                                    hurt.Play()
                                End If
                                health -= 5
                                statDamage += 5
                            End If
                        End If
                    End If
                End If
            Next counter

            'Control ruin collisions
            For counter As Integer = 0 To ruins.Count
                If counter < ruins.Count Then
                    ruins(counter).CollideAgainstMove(character, 1, 0)
                End If
            Next counter

            If frameSkip = 10 Then
                'Control enemies collisions with eachother
                For counter As Integer = 0 To enemies.Count - 1
                    For counter2 As Integer = counter + 1 To enemies.Count - 1
                        If enemies(counter).ScaleX = 1 And enemies(counter2).ScaleX = 1 Then
                            enemies(counter).CollideAgainstMove(enemies(counter2), 1, 1)
                        End If
                    Next counter2
                Next counter
                frameSkip = 0
            End If

            'Control enemies collisions with ruins
            For counter As Integer = 0 To ruins.Count
                For counter2 As Integer = 0 To enemies.Count
                    If counter < ruins.Count And counter2 < enemies.Count Then
                        ruins(counter).CollideAgainstMove(enemies(counter2), 1, 0)
                    End If
                Next counter2
            Next counter

            'Control tower collisions
            For counter As Integer = 0 To towers.Count
                If counter < towers.Count Then
                    If towers(counter).CollideAgainstMove(character, 1, 0) = True Then
                        If superSlash = True Then
                            towerDestroy.Play()
                            Dim drop As Sprite
                            drop = SpriteManager.AddSprite("Content\\images\\tower_broken.png")
                            drop.X = towers(counter).X
                            drop.Y = towers(counter).Y
                            drop.ScaleX = towers(counter).ScaleX
                            drop.ScaleY = towers(counter).ScaleY
                            rubbish.Add(drop)
                            SpriteManager.RemoveSprite(towers(counter))
                            energy -= 300
                            power -= 300
                        End If
                    End If
                End If
            Next counter

            'Control enemies collisions with towers
            For counter As Integer = 0 To towers.Count
                For counter2 As Integer = 0 To enemies.Count
                    If counter < towers.Count And counter2 < enemies.Count Then
                        towers(counter).CollideAgainstMove(enemies(counter2), 1, 0)
                    End If
                Next counter2
            Next counter

            'Control powerup collision
            For counter As Integer = 0 To powerups.Count - 1
                If counter < powerups.Count Then
                    If powerups(counter).CollideAgainst(character) Then
                        If powerupsType(counter) = 0 Then
                            health += 500
                        ElseIf powerupsType(counter) = 1 Then
                            energyRegen += 1
                        ElseIf powerupsType(counter) = 2 Then
                            extraSpeed += 1
                        ElseIf powerupsType(counter) = 3 Then
                            energy += 500
                        End If
                        SpriteManager.RemoveSprite(powerups(counter))
                        powerupsType.RemoveAt(counter)
                    End If
                End If
            Next counter


            ''Camera
            'Move camera with character
            If character.X > leftEdge And character.X < rightEdge Then
                SpriteManager.Cameras(0).X = character.X
            End If

            If character.Y > bottomEdge And character.Y < topEdge Then
                SpriteManager.Cameras(0).Y = character.Y
            End If


            ''Player
            'Check for player control of the player
            If tool.isKeyPressed(Keys.W) And character.Y < 50 Or tool.isKeyPressed(Keys.Up) And character.Y < 50 Then
                character.YVelocity = speed + extraSpeed
                If tool.random(0, 30) = 0 Then
                    walk.Play(0.4, 0, 0)
                End If
            ElseIf tool.isKeyPressed(Keys.S) And character.Y > -55 Or tool.isKeyPressed(Keys.Down) And character.Y > -55 Then
                character.YVelocity = (speed + extraSpeed) * -1
                If tool.random(0, 30) = 0 Then
                    walk.Play(0.4, 0, 0)
                End If
            Else
                character.YVelocity = 0
            End If

            If tool.isKeyPressed(Keys.D) And character.X < 70 Or tool.isKeyPressed(Keys.Right) And character.X < 70 Then
                character.XVelocity = speed + extraSpeed
                character.Texture = tool.changeTexture("Content\\images\\character_r.png")
                direction = 0
                If tool.random(0, 30) = 0 Then
                    walk.Play(0.4, 0, 0)
                End If
            ElseIf tool.isKeyPressed(Keys.A) And character.X > -70 Or tool.isKeyPressed(Keys.Left) And character.X > -70 Then
                character.XVelocity = (speed + extraSpeed) * -1
                character.Texture = tool.changeTexture("Content\\images\\character_l.png")
                direction = 1
                If tool.random(0, 30) = 0 Then
                    walk.Play(0.4, 0, 0)
                End If
            Else
                character.XVelocity = 0
            End If

            'Supermove
            If energy > 299 And power > 299 Then
                superReady.Visible = True
                If tool.isKeyPressed(Keys.E) Then
                    superSlash = True
                Else
                    superSlash = False
                End If
            Else
                superReady.Visible = False
                superSlash = False
            End If

            'Slash with sword
            If tool.isKeyPushed(Keys.Space) And energy > 3 Then
                sword.Play()
            End If

            If tool.isKeyPressed(Keys.Space) And energy > 3 Then
                energy -= 4
                If direction = 0 Then
                    character.Texture = tool.changeTexture("Content\\images\\character_r_slash.png")
                    slash = True
                Else
                    character.Texture = tool.changeTexture("Content\\images\\character_l_slash.png")
                    slash = True
                End If
            Else
                If direction = 0 Then
                    If superSlash = True Then
                        character.Texture = tool.changeTexture("Content\\images\\character_r_hammer_up.png")
                    Else
                        character.Texture = tool.changeTexture("Content\\images\\character_r.png")
                    End If
                    slash = False
                Else
                    If superSlash = True Then
                        character.Texture = tool.changeTexture("Content\\images\\character_l_hammer_up.png")
                    Else
                        character.Texture = tool.changeTexture("Content\\images\\character_l.png")
                    End If
                    slash = False
                End If
            End If

            'Prevent players from holding space bar down once energy is empty
            If tool.isKeyPressed(Keys.Space) And energy < 4 Then
                energy = -1
            End If

            'Sprint
            If tool.isKeyPressed(Keys.LeftShift) And energy > 0 Then
                speed = 15
                energy -= 3
            Else
                speed = 10
            End If

            'Regernerate Energy
            If energy <= 1000 - energyRegen Then
                energy += energyRegen
                statEnergy += energyRegen
            End If

            ''Powerups
            'Drop powerups
            If tool.isKeyPushed(Keys.D1) And power > 799 And energyRegen < 3 Then
                DropPowerup(1, "Content\\images\\parachute_energy_regen.png", 800)
            ElseIf tool.isKeyPushed(Keys.D2) And power > 399 Then
                DropPowerup(0, "Content\\images\\parachute_health.png", 400)
            ElseIf tool.isKeyPushed(Keys.D3) And power > 399 Then
                DropPowerup(3, "Content\\images\\parachute_energy.png", 400)
            ElseIf tool.isKeyPushed(Keys.D4) And power > 599 Then
                DropPowerup(2, "Content\\images\\parachute_speed.png", 600)
            End If

            'Control powerup pickup
            For counter As Integer = 0 To powerups.Count
                If counter < powerups.Count Then
                    If powerups(counter).YVelocity < 0 And powerups(counter).Y < character.Y Then
                        powerups(counter).YVelocity = 0
                        powerups(counter).ScaleY = 1
                        powerups(counter).ScaleX = 1
                        powerups(counter).Y -= 2.8
                        drop.Play()
                        If powerupsType(counter) = 0 Then
                            powerups(counter).Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\box_health.png")
                        ElseIf powerupsType(counter) = 1 Then
                            powerups(counter).Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\box_energy_regen.png")
                        ElseIf powerupsType(counter) = 2 Then
                            powerups(counter).Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\box_speed.png")
                        ElseIf powerupsType(counter) = 3 Then
                            powerups(counter).Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\box_energy.png")
                        End If
                    End If
                End If
            Next counter


            ''Spawning
            'Count to spawnSpeed ticks then spawn a slime at each tower
            If spawnCounter < spawnSpeed Then
                spawnCounter += 1
                If spawnCounter = spawnSpeed - 30 Then
                    For counter As Integer = 0 To towers.Count
                        If counter < towers.Count Then
                            towers(counter).Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\tower_open.png")
                            gateOpen.Play(0.2, 0, 0)
                        End If
                    Next (counter)
                ElseIf spawnCounter = 30 Then
                    For counter As Integer = 0 To towers.Count
                        If counter < towers.Count Then
                            towers(counter).Texture = FlatRedBallServices.Load(Of Texture2D)("Content\\images\\tower.png")
                            gateClose.Play(0.2, 0, 0)
                        End If
                    Next (counter)
                End If
            Else
                spawnCounter = 0
                For counter As Integer = 0 To towers.Count
                    If counter < towers.Count Then

                        Dim enemy As Sprite
                        Dim type As Integer
                        type = tool.random(0, 3)

                        If type = 0 Then
                            enemy = SpriteManager.AddSprite(blobAnimation)
                        ElseIf type = 1 Then
                            enemy = SpriteManager.AddSprite(blobAnimation2)
                        ElseIf type = 2 Then
                            enemy = SpriteManager.AddSprite(blobAnimation3)
                        Else
                            enemy = SpriteManager.AddSprite(blobAnimation)
                        End If
                        enemy.Z = 0.1
                        enemy.X = towers(counter).X
                        enemy.Y = towers(counter).Y - 7
                        tool.mapCollisionRectangle(enemy)
                        enemies.Add(enemy)
                        slime.Play()
                    End If
                Next counter
            End If

            '"win" level (or so they think)
            If towers.Count < 1 And enemies.Count < 1 And won = False Then
                'Change Track
                music = FlatRedBallServices.Load(Of Song)("Content\\sounds\\boss", ContentManagerName)
                Microsoft.Xna.Framework.Media.MediaPlayer.Play(music)
                MediaPlayer.IsRepeating = True
                'Setup boss
                won = True
                bossHealth = 1000
                boss = SpriteManager.AddSprite(bossAnimation)
                tool.mapCollisionRectangle(boss)
                boss.Z = 0.2
                enemies.Add(boss)
            End If

            'Run boss updates
            If won = True Then
                boss.ScaleX = bossHealth / 100
                boss.ScaleY = bossHealth / 100
                tool.mapCollisionRectangle(boss)
            End If

            'Win level
            If won = True And enemies.Count < 1 Then
                CustomDestroy()
                MoveToScreen(GetType(gameResults).FullName)
            End If

            'Lose level
            If health < 1 Then
                CustomDestroy()
                MoveToScreen(GetType(die).FullName)
            End If
        End Sub

        Private Sub DropPowerup(ByVal powerupType As Integer, ByVal image As String, ByVal powerCost As Integer)
            wind.Play()
            Dim powerup As Sprite
            powerup = SpriteManager.AddSprite(image)
            powerup.ScaleY = 4
            powerup.ScaleX = 3
            powerup.X = character.X
            powerup.Y = character.Y + 20
            powerup.Z = 0.5
            powerup.YVelocity = -6
            tool.mapCollisionRectangle(powerup)
            powerups.Add(powerup)
            powerupsType.Add(powerupType)
            power -= powerCost
        End Sub

        Private Sub CustomDestroy()
            SpriteManager.RemoveSpriteList(ruins)
            SpriteManager.RemoveSpriteList(towers)
            SpriteManager.RemoveSpriteList(rubbish)
            SpriteManager.RemoveSpriteList(enemies)
            SpriteManager.RemoveSpriteList(powerups)
            SpriteManager.RemoveSprite(character)
            SpriteManager.RemoveSprite(healthDraw)
            SpriteManager.RemoveSprite(energyDraw)
            SpriteManager.RemoveSprite(powerDraw)
            SpriteManager.RemoveSprite(superReady)
            SpriteManager.Cameras.Remove(camera_a)
            SpriteManager.Cameras.Remove(camera_hud)
            TextManager.RemoveText(healthText)
            TextManager.RemoveText(powerText)
            TextManager.RemoveText(energyText)
            SpriteManager.RemoveScene(level1, True)
        End Sub

        Private Shared Sub CustomLoadStaticContent(ByVal contentManagerName As String)

        End Sub

    End Class
End Namespace
