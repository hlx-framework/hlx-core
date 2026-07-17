// This file exists so CI can still typecheck every module in the haxelib by importing them all with an empty main.
import HlxRuntime;
import hlx.runtime.HlxPrefixControl;
import hlx.runtime.HlxPrefixResult;
import hlx.runtime.HookDiscoveryMacro;
import hlx.runtime.Mod;
import hlx.runtime.ModConfig;
import hlx.runtime.ModConfigMacro;
import hlx.runtime.PatchTargetKey;
import hlx.runtime.ResolvedMember;

class Main {
	static function main() {}
}
