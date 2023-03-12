import Atom

class Player(Atom.Entity):
    jump_force: float
    movement_speed: float
    rotation_speed: float
    camera_holder: Atom.Entity
    _vertical_force: float
    _is_in_air: bool
    _velocity: Atom.Vec3
    _prev_mouse_pos: Atom.Vec2
    _animation_controller: Atom.AnimationController

    def __init__(self, entity_id: int) -> None:
        super().__init__(entity_id)
        self.jump_force = 5.0
        self.movement_speed = 7.0
        self.rotation_speed = 7.0
        self.camera_holder = Atom.Entity(0)
        self._vertical_force = 0.0
        self._is_in_air = False
        self._velocity = Atom.Vec3(0.0, 0.0, 0.0)
        self._prev_mouse_pos = Atom.Vec2(0.0, 0.0)

    def on_create(self) -> None:
        self._prev_mouse_pos = Atom.Input.get_mouse_position()
        self._animation_controller = self.get_animator_component().animation_controller
        Atom.Input.set_mouse_cursor(False)

    def on_update(self, ts: Atom.Timestep) -> None:
        if Atom.Input.is_cursor_enabled():
            return
        
        # Compute rotations
        current_mouse_pos: Atom.Vec2 = Atom.Input.get_mouse_position()
        mouse_delta: Atom.Vec2 = current_mouse_pos - self._prev_mouse_pos
        Atom.Input.set_mouse_position(self._prev_mouse_pos)
        
        self.camera_holder.transform.rotation.x -= Atom.Math.radians(mouse_delta.y * self.rotation_speed * ts.get_seconds())
        self.camera_holder.transform.rotation.x = Atom.Math.clamp(self.camera_holder.transform.rotation.x, Atom.Math.radians(-80.0), Atom.Math.radians(80.0))

        self.transform.rotation.y -= Atom.Math.radians(mouse_delta.x * self.rotation_speed * ts.get_seconds())

        # Compute directional movement
        self._velocity = Atom.Vec3(0.0, 0.0, 0.0)

        forward_movement: int = 0
        side_movement: int = 0

        if Atom.Input.is_key_pressed(Atom.Key.W):
            self._velocity += self.transform.forward_vector * self.movement_speed * ts.get_seconds()
            forward_movement = 1
        elif Atom.Input.is_key_pressed(Atom.Key.S):
            self._velocity -= self.transform.forward_vector * self.movement_speed * ts.get_seconds()
            forward_movement = -1

        if Atom.Input.is_key_pressed(Atom.Key.A):
            self._velocity -= self.transform.right_vector * self.movement_speed * ts.get_seconds()
            side_movement = -1
        elif Atom.Input.is_key_pressed(Atom.Key.D):
            self._velocity += self.transform.right_vector * self.movement_speed * ts.get_seconds()
            side_movement = 1

        if self._vertical_force > -9.8:
            self._vertical_force += -9.8 * ts.get_seconds()
            
        self._velocity += self.transform.up_vector * self._vertical_force * ts.get_seconds()
        self.transform.translation += self._velocity

        if self.transform.translation.y <= 0.0:
            self._is_in_air = False
            self._velocity.y = 0.0
            self.transform.translation.y = 0.0

        if not self._is_in_air:
            if forward_movement == 1 and side_movement == 1:
                # Forward Right
                self._animation_controller.transition_to_state(1)
            elif forward_movement == 1 and side_movement == 0:
                # Forward
                self._animation_controller.transition_to_state(2)
            elif forward_movement == 1 and side_movement == -1:
                # Forward Left
                self._animation_controller.transition_to_state(3)
            elif forward_movement == 0 and side_movement == -1:
                # Left
                self._animation_controller.transition_to_state(4)
            elif forward_movement == -1 and side_movement == -1:
                # Backwards Left
                self._animation_controller.transition_to_state(5)
            elif forward_movement == -1 and side_movement == 0:
                # Backwards
                self._animation_controller.transition_to_state(6)
            elif forward_movement == -1 and side_movement == 1:
                # Backwards Right
                self._animation_controller.transition_to_state(7)
            elif forward_movement == 0 and side_movement == 1:
                # Right
                self._animation_controller.transition_to_state(8)
            else:
                # Idle
                self._animation_controller.transition_to_state(0)

    def on_event(self, event: Atom.Event) -> None:
        if isinstance(event, Atom.MouseButtonPressedEvent):
            if event.mouse_button == Atom.MouseButton.Right:
                Atom.Input.set_mouse_cursor(False)
        elif isinstance(event, Atom.KeyPressedEvent):
            if event.key == Atom.Key.Esc:
                Atom.Input.set_mouse_cursor(True)
            elif event.key == Atom.Key.Space:
                self.jump()

    def jump(self) -> None:
        if not self._is_in_air:
            self._is_in_air = True
            self._vertical_force = self.jump_force
            self.get_animator_component().current_time = 0.0
            self._animation_controller.transition_to_state(9)