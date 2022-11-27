import Atom

class Camera(Atom.Entity):
    Player: Atom.Entity

    def __init__(self, entityID: int):
        super().__init__(entityID)
        self.Player = Atom.Entity(0)

    def on_create(self):
        Atom.Log.info("Camera::OnCreate")

    def on_update(self, ts: Atom.Timestep):
        if self.Player.is_valid():
            self.get_transform_component().translation = Atom.Vec3(self.Player.get_transform_component().translation.x, self.Player.get_transform_component().translation.y, self.get_transform_component().translation.z)

