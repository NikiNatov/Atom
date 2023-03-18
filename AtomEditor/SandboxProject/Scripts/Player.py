import Atom
import math

class PlayerRigidbody(Atom.Entity):
    jump_force: float
    movement_speed: float
    rotation_speed: float
    camera_holder: Atom.Entity
    _movement_input: Atom.Vec3
    _mouse_delta: Atom.Vec2
    _prev_mouse_pos: Atom.Vec2
    _animation_controller: Atom.AnimationController
    _rigidbody: Atom.RigidbodyComponent

    def __init__(self, entity_id: int) -> None:
        super().__init__(entity_id)
        self.jump_force = 5.0
        self.movement_speed = 7.0
        self.rotation_speed = 7.0
        self.camera_holder = Atom.Entity(0)
        self._movement_input = Atom.Vec3(0.0, 0.0, 0.0)
        self._mouse_delta = Atom.Vec2(0.0, 0.0)
        self._prev_mouse_pos = Atom.Vec2(0.0, 0.0)
        self._rigidbody = Atom.RigidbodyComponent()

    def on_create(self) -> None:
        self._prev_mouse_pos = Atom.Input.get_mouse_position()
        self._animation_controller = self.get_animator_component().animation_controller
        self._rigidbody = self.get_rigidbody_component()
        Atom.Input.set_mouse_cursor(False)

    def on_fixed_update(self, ts: Atom.Timestep) -> None:
        # Rotate player
        yRotationAngle: float = Atom.Math.radians(-self._mouse_delta.x * self.rotation_speed * ts.get_seconds())
        self._rigidbody.rotation *= Atom.Math.angle_axis(yRotationAngle, Atom.Vec3(0.0, 1.0, 0.0))

        # Move player
        velocity: Atom.Vec3 = Atom.Vec3(0.0, 0.0, 0.0)

        velocity += self._rigidbody.forward_vector * self._movement_input.z * self.movement_speed * ts.get_seconds()
        velocity += self._rigidbody.right_vector * self._movement_input.x * self.movement_speed * ts.get_seconds()

        self._rigidbody.translation += velocity

        # Set animation state
        if self._movement_input.z == 1 and self._movement_input.x == 1:
            # Forward Right
            self._animation_controller.transition_to_state(1)
        elif self._movement_input.z == 1 and self._movement_input.x == 0:
            # Forward
            self._animation_controller.transition_to_state(2)
        elif self._movement_input.z == 1 and self._movement_input.x == -1:
            # Forward Left
            self._animation_controller.transition_to_state(3)
        elif self._movement_input.z == 0 and self._movement_input.x == -1:
            # Left
            self._animation_controller.transition_to_state(4)
        elif self._movement_input.z == -1 and self._movement_input.x == -1:
            # Backwards Left
            self._animation_controller.transition_to_state(5)
        elif self._movement_input.z == -1 and self._movement_input.x == 0:
            # Backwards
            self._animation_controller.transition_to_state(6)
        elif self._movement_input.z == -1 and self._movement_input.x == 1:
            # Backwards Right
            self._animation_controller.transition_to_state(7)
        elif self._movement_input.z == 0 and self._movement_input.x == 1:
            # Right
            self._animation_controller.transition_to_state(8)
        else:
            # Idle
            self._animation_controller.transition_to_state(0)

    def on_update(self, ts: Atom.Timestep) -> None:
        if Atom.Input.is_cursor_enabled():
            return
        
        # Get mouse inputs
        current_mouse_pos: Atom.Vec2 = Atom.Input.get_mouse_position()
        self._mouse_delta = current_mouse_pos - self._prev_mouse_pos
        Atom.Input.set_mouse_position(self._prev_mouse_pos)
        
        # Get key inputs
        self._movement_input = Atom.Vec3(0.0, 0.0, 0.0)
        
        if Atom.Input.is_key_pressed(Atom.Key.W):
            self._movement_input.z = 1
        elif Atom.Input.is_key_pressed(Atom.Key.S):
            self._movement_input.z = -1

        if Atom.Input.is_key_pressed(Atom.Key.A):
            self._movement_input.x = -1
        elif Atom.Input.is_key_pressed(Atom.Key.D):
            self._movement_input.x = 1

        # Camera rotations
        xRotationAngle: float = Atom.Math.radians(-self._mouse_delta.y * self.rotation_speed * ts.get_seconds())
        if self.camera_holder.transform.euler_angles.x + xRotationAngle > -math.pi / 2.0 and self.camera_holder.transform.euler_angles.x + xRotationAngle < math.pi / 2.0:
            self.camera_holder.transform.rotation *= Atom.Math.angle_axis(xRotationAngle, Atom.Vec3(1.0, 0.0, 0.0))

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
        self._rigidbody.add_impulse(Atom.Vec3(0.0, self.jump_force, 0.0), True)
        self.get_animator_component().current_time = 0.0
        self._animation_controller.transition_to_state(9)