using System.Diagnostics.Contracts;
using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.Memory;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;
using CTFAK.Utils;

public class GlobalStoreXExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "RTSD";
	public override string ExtensionName => "gstorex";
	public override string CppClassName => "GlobalStoreXExtension";

	public override string ExportExtension(byte[] extensionData)
	{
		ByteReader reader = new ByteReader(extensionData);
		reader.Skip(8); // 8 bytes unknown

		uint IntegerCount = reader.ReadUInt32();
		uint StringCount = reader.ReadUInt32();
		uint BoolCount = reader.ReadUInt32();
		uint ShortCount = reader.ReadUInt32();

		int flags = 0;
		if (!reader.ReadBoolean()) flags |= 0x01; // integer 
		if (!reader.ReadBoolean()) flags |= 0x02; // string 
		if (!reader.ReadBoolean()) flags |= 0x04; // bool 
		if (!reader.ReadBoolean()) flags |= 0x08; // short 

		return CreateExtension($"{IntegerCount}, {StringCount}, {BoolCount}, {ShortCount}, {flags}");
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (conditionNum)
		{
			case -81: // Compare Integer
				result.AppendLine($"{ifStatement} ({GetExtensionInstance(eventBase.ObjectInfo)}->GetInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}) {ExpressionConverter.GetComparisonSymbol(((ExpressionParameter)eventBase.Items[1].Loader).Comparsion)} {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)})) goto {nextLabel};");
				break;
			case -82: // Compare Bool
				result.AppendLine($"{ifStatement} ({GetExtensionInstance(eventBase.ObjectInfo)}->GetBool({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}) {ExpressionConverter.GetComparisonSymbol(((ExpressionParameter)eventBase.Items[1].Loader).Comparsion)} {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)})) goto {nextLabel};");
				break;
			case -83: // Compare Short
				result.AppendLine($"{ifStatement} ({GetExtensionInstance(eventBase.ObjectInfo)}->GetShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}) {ExpressionConverter.GetComparisonSymbol(((ExpressionParameter)eventBase.Items[1].Loader).Comparsion)} {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)})) goto {nextLabel};");
				break;
			default:
				result.AppendLine($"// Global Store X condition {conditionNum} not implemented");
				result.AppendLine($"goto {nextLabel};");
				break;
		}

		return result.ToString();
	}

	public override string ExportAction(EventBase eventBase, int actionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (actionNum)
		{
			case 80: // Set Integer
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SetInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 81: // Set Bool
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SetBool({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 82: // Set String
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SetString({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 83: // Add Integer
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->AddInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 84: // Set Bool Range
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SetRangeBool({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[2].Loader, eventBase)});");
				break;
			case 85: // Add String
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->AddString({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 86: // Subtract Integer
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SubtractInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 87: // Clear Bools
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ClearBools();");
				break;
			case 88: // Set Integer Range
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SetRangeInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[2].Loader, eventBase)});");
				break;
			case 89: // Set String Range
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SetRangeString({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[2].Loader, eventBase)});");
				break;
			case 90: // Clear Integers
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ClearIntegers();");
				break;
			case 91: // Resize Integers
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ResizeIntegers({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 92: // Expand Integers
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ExpandIntegers({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 93: // Clear Strings
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ClearStrings();");
				break;
			case 94: // Resize Strings
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ResizeStrings({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 95: // Expand Strings
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ExpandStrings({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 96: // Resize Bools
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ResizeBools({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 97: // Expand Bools
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ExpandBools({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 98: // Save Integer INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveIntegerINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 99: // Save String INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveStringINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 100: // Save Bool INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveBoolINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 101: // Save All INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveAllINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 102: // Load Integer INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadIntegerINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 103: // Load String INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadStringINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 104: // Load Bool INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadBoolINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 105: // Load All INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadAllINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 106: // Save Integer Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveIntegerBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 107: // Save String Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveStringBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 108: // Save Bool Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveBoolBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 109: // Save All Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveAllBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 110: // Load Integer Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadIntegerBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 111: // Load String Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadStringBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 112: // Load Bool Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadBoolBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 113: // Load All Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadAllBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 114: // Set Short
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SetShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 115: // Add Short
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->AddShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 116: // Subtract Short
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SubtractShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 117: // Set Short Range
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SetRangeShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[2].Loader, eventBase)});");
				break;
			case 118: // Clear Shorts
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ClearShorts();");
				break;
			case 119: // Resize Shorts
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ResizeShorts({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 120: // Expand Shorts
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->ExpandShorts({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 121: // Save Short INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveShortINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 122: // Load Short INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadShortINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 123: // Save Short Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->SaveShortBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 124: // Load Short Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo)}->LoadShortBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			default:
				result.AppendLine($"// Global Store X action {actionNum} not implemented");
				break;
		}

		return result.ToString();
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case 80: // Get Integer
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetInteger(";
				break;
			case 81: // Get Bool
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetBool(";
				break;
			case 82: // Get String
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetString(";
				break;
			case 83: // Get Integer Array Size
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetIntegerArraySize()";
				break;
			case 84: // Get String Array Size
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetStringArraySize()";
				break;
			case 85: // Get Integer Base
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetIntegerBase()";
				break;
			case 86: // Get String Base
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetStringBase()";
				break;
			case 87: // Get Bool Size
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetBoolArraySize()";
				break;
			case 88: // Get Bool Base
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetBoolBase()";
				break;
			case 90: // Get Short Size
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetShortArraySize()";
				break;
			case 91: // Get Short Base
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetShortBase()";
				break;
			case 89: // Get Short
				result = $"{GetExtensionInstance(expression.ObjectInfo)}->GetShort(";
				break;
			default:
				result = $"0 /* Global Store X expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}