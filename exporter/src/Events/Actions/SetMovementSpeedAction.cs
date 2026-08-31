using System.Text;
using CTFAK.CCN.Chunks.Frame;

public class SetMovementSpeedAction : ActionBase
{
	public override int[] ObjectType { get; set; } = [2, 3, 7];
	public override int Num { get; set; } = 6;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		StringBuilder result = new StringBuilder();

		result.AppendLine($"for (ObjectIterator it({GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}); !it.end(); ++it) {{");
		result.AppendLine($"    auto instance = *it;");
		result.AppendLine($"    (({ExpressionConverter.GetObjectClassName(eventBase.ObjectInfo, eventBase.ObjectType)}*)instance)->movements.GetCurrentMovement()->SetCurrentSpeed(({ConvertExpression(eventBase, 0)}).GetIntValue());");
		result.AppendLine("}");

		return result.ToString();
	}
}
