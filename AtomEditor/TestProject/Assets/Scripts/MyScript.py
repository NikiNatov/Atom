import Atom

class MyScript(Atom.Entity):
    MyInt: int
    MyFloat: float
    MyBool: bool
    MyVec2: Atom.Vec2

    def __init__(self, entityID):
        super().__init__(entityID)
        self.MyInt = 5
        self.MyFloat = 6.0
        self.MyBool = True
        self.MyVec2 = Atom.Vec2(4.0, 3.0)

    def on_create(self):
        Atom.Log.Info("MyScript::OnCreate")

    def on_update(self, ts: float):
        transformComponent = self.get_transform_component();
        Atom.Log.Info("x = {}, y = {}, z = {}".format(transformComponent.translation.x, transformComponent.translation.y, transformComponent.translation.z))


