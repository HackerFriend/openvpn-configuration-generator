// Copyright SparkLabs Pty Ltd 2018

#pragma once

#include "stdafx.h"

#include <iostream>
#include "CLI.h"
#include "Interactive.h"

using namespace std;
using namespace System;
using namespace System::IO;

int main(int argc, char *argv[], char *envp[])
{
	CLI^ cli = gcnew CLI();

	//Enough args?
	if (argc < 2) {
		cli->printUsage();
		System::Environment::Exit(1);
	}

	//Get Mode
	String^ modeStr = gcnew String(argv[1]);
	CLI::Mode mode = cli->getMode(modeStr);

	if (mode == CLI::Mode::Help) {
		cli->printUsage();
		System::Environment::Exit(0);
	}
	if (mode == CLI::Mode::About) {
		cli->printAbout();
		System::Environment::Exit(0);
	}

	//Parse Options
	Dictionary<CLI::OptionType, String^>^ options = gcnew Dictionary<CLI::OptionType, String^>((int)CLI::OptionType::Unknown);

	for (int i = 2; i < argc; i++) {
		String^ opStr = gcnew String(argv[i]);
		CLI::OptionType op = cli->getOption(opStr);
		i++;
		if (op == CLI::OptionType::Unknown) {
			Console::WriteLine("Unknown Option. Exiting");
			cli->printUsage();
			System::Environment::Exit(1);
		}
		if (i < argc) {
			String^ opVal = gcnew String(argv[i]);
			options[op] = opVal;
		}
		else {
			Console::WriteLine("Option missing argument. Exiting");
			cli->printUsage();
			System::Environment::Exit(1);
		}
	}

	if (mode == CLI::Mode::Unknown) {
		Console::WriteLine("Unknown Mode");
		cli->printUsage();
		System::Environment::Exit(1);
	}

	//Determine the path
	String^ path;
	if (options->ContainsKey(CLI::OptionType::Path)) {
		path = options[CLI::OptionType::Path];
	}
	else {
		// Get working dir
		path = Directory::GetCurrentDirectory();
	}


	//Determine path exists
	if (!Directory::Exists(path)) {
		Console::WriteLine("Path {0} not found.", path);
		System::Environment::Exit(1);
	}

	//Init SSL
	OpenSSLHelper::OpenSSL_INIT();

	if (mode == CLI::Mode::InitSetup) {
		int keySize;
		int validDays;
		String^ kSize;
		if (options->TryGetValue(CLI::OptionType::KeySize, kSize)) {
			//Conver to int
			if (!int::TryParse(kSize, keySize)) {
				Console::WriteLine("Key Size is not valid");
				Environment::Exit(1);
			}
		}
		else {
			keySize = 2048;
		}
		String^ vDays;
		if (options->TryGetValue(CLI::OptionType::ValidDays, vDays)) {
			//Conver to int
			if (!int::TryParse(vDays, validDays)) {
				Console::WriteLine("Valid Days is not valid");
				Environment::Exit(1);
			}
		}
		else {
			validDays = 3650;
		}

		Interactive^ interactive = gcnew Interactive(path, keySize, validDays);
		if (!interactive->GenerateNewConfig())
			Environment::Exit(1);
		Console::WriteLine();
		Console::WriteLine("Generating new server configuration...");
		if (!interactive->CreateNewIssuer())
			Environment::Exit(1);
		if (!interactive->CreateDH())
			Environment::Exit(1);
		if (!interactive->CreateServerConfig())
			Environment::Exit(1);
		if (!interactive->SaveConfig())
			Environment::Exit(1);

		Console::WriteLine("Successfully initialised config.");
		Environment::Exit(0);
	}
	else if (mode == CLI::Mode::CreateClient) {
		Interactive^ interactive = gcnew Interactive(path, 1, 1);
		if (!interactive->LoadConfig())
			Environment::Exit(1);

		String^ name;
		if (!options->TryGetValue(CLI::OptionType::CommonName, name)) {
			name = nullptr;
		}

		if (!interactive->CreateNewClientConfig(name))
			Environment::Exit(1);
		if (!interactive->SaveConfig())
			Environment::Exit(1);

		Console::WriteLine("Successfully created new client.");
		Environment::Exit(0);
	}
	else {
		//In theory, this should never be hit...
		Console::WriteLine("Unknown Mode");
		cli->printUsage();
		System::Environment::Exit(1);
	}
}

