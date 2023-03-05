import Atom

class Camera(Atom.Entity):
    movement_speed: float
    rotation_speed: float
    _prev_mouse_pos: Atom.Vec2
    _velocity: Atom.Vec3

    def __init__(self, entityID: int):
        super().__init__(entityID)
        self.movement_speed = 5.0
        self.rotation_speed = 5.0
        self._prev_mouse_pos = Atom.Vec2(0.0, 0.0)
        self._velocity = Atom.Vec3(0.0, 0.0, 0.0)

    def on_create(self) -> None:
        self._prev_mouse_pos = Atom.Input.get_mouse_position()
        Atom.Input.set_mouse_cursor(False)

    def on_destroy(self) -> None:
        Atom.Input.set_mouse_cursor(True)

    def on_update(self, ts: Atom.Timestep) -> None:
        if Atom.Input.is_cursor_enabled():
            return
        
        # Compute rotations
        current_mouse_pos: Atom.Vec2 = Atom.Input.get_mouse_position()
        mouse_delta: Atom.Vec2 = current_mouse_pos - self._prev_mouse_pos
        Atom.Input.set_mouse_position(self._prev_mouse_pos)
        
        self.transform.rotation.x -= Atom.Math.radians(mouse_delta.y * self.rotation_speed * ts.get_seconds())

        if self.transform.rotation.x > Atom.Math.radians(89.0):
            self.transform.rotation.x = Atom.Math.radians(89.0)
        elif self.transform.rotation.x < Atom.Math.radians(-80.0):
            self.transform.rotation.x = Atom.Math.radians(-80.0)

        self.transform.rotation.y -= Atom.Math.radians(mouse_delta.x * self.rotation_speed * ts.get_seconds())

        # Compute directional movement
        self._velocity = Atom.Vec3(0.0, 0.0, 0.0)

        if Atom.Input.is_key_pressed(Atom.Key.W):
            self._velocity += self.transform.forward_vector * self.movement_speed * ts.get_seconds()
        elif Atom.Input.is_key_pressed(Atom.Key.S):
            self._velocity -= self.transform.forward_vector * self.movement_speed * ts.get_seconds()

        if Atom.Input.is_key_pressed(Atom.Key.A):
            self._velocity -= self.transform.right_vector * self.movement_speed * ts.get_seconds()
        elif Atom.Input.is_key_pressed(Atom.Key.D):
            self._velocity += self.transform.right_vector * self.movement_speed * ts.get_seconds()

        if Atom.Input.is_key_pressed(Atom.Key.Space):
            self._velocity += self.transform.up_vector * self.movement_speed * ts.get_seconds()
        elif Atom.Input.is_key_pressed(Atom.Key.LShift):
            self._velocity -= self.transform.up_vector * self.movement_speed * ts.get_seconds()

        self.transform.translation += self._velocity

    def on_event(self, event: Atom.Event) -> None:
        if isinstance(event, Atom.KeyPressedEvent):
            if event.key == Atom.Key.Esc:
                Atom.Input.set_mouse_cursor(True)