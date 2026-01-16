// --------------------------------------------------------------------
// \file    helpndoc.pas
// \autor   (c) 2025, 2026 by Jens Kallup - paule32
// \copy    all rights reserved.
//
// \details HelpNDoc PascalScript - One-shot importer Supports: /** */
//          and /// blocks
//          Creates topics and fills HTML using Temporary Editor.
// \note    supported tags:
//
// /**
//   * @brief  reserve memory from the kernel heap.
//   * @param  size count bytes.
//   * @return Pointer of block or NULL.
//   * @note   use it only in kernel-context.
//   * @see    kfree
//   */
// --------------------------------------------------------------------

// --------------------------------------------------------------------
// Request to HelpNDoc.com:
// for interactive use with the document writer, it makes sense to have
// the possiblity to select the ROOT_DIR and ROOT_HND with a dialog box
// that supports input line for the file name and selecting the direct-
// ory in which the HND file will be created.
// in this context i would like to have a confirm box with 3 buttons to
// select (Yes - overwrite file) / (NO - don't overwrite) (CAMCEL - to
// cancel the executin of the Pascal Script).
// --------------------------------------------------------------------
const ROOT_DIR = 'T:\a\WelcomeBackOS\docs\html'; // <-- customize
const ROOT_HND = ROOT_DIR +      '\default.hnd';

const ROOT_HND_DEL = 1;               // del default.hnd before save ?

const INCLUDE_CPP  = False;           // True, if .c/.cpp auch
const ONLY_HEADERS = True;            // True: only .h/.hpp

const HelpNDocTemplateHTM = 'template.htm';

type
  TStringArray = TStringList;

type DWORD = Integer;

type
  TDocItem = record
    Kind      : string;     // "function" | "type"
    Name      : string;     // symbol name
	
    Signature : string;     // extracted signature line(s)
    GroupName : string;     // from @ingroup or "General"
    Brief     : string;
    ReturnText: string;
	runningPDF: boolean;
    
	Notes     : TStringArray;
    Warnings  : TStringArray;
    Sees      : TStringArray;
    ParamNames: TStringArray;
    ParamTexts: TStringArray;
	
    SourceFile: string;
    LineNo    : Integer;
  end;

var currentTopic: String = '';

type
  TEditor = class(TObject)
  private
    ID: TObject;
    Content: String;
  public
    constructor Create;
    destructor Destroy; override;
    
    procedure Clear;

    procedure LoadFromFile(AFileName: String);
    procedure LoadFromString(AString: String);
    
    procedure SaveToFile(AFileName: String);
    
    function  getContent: String;
    function  getID: TObject;
    
    procedure setContent(AString: String);
  end;

type
  THtmlContent = class(TObject)
  private
    Fhtml: TStringList;
  public
    constructor Create;
    destructor Destroy; override;
  end;

type
  TContentStyle = class(TObject)
  private
    styleContent: TStringList;
  public
    constructor Create(AStyle: String); overload;
    constructor Create; overload;
    destructor Destroy; override;
  end;

type
  TTopicContentDIV = class(THtmlContent)
  private
    divContent: TStringList;
    divClass  : TStringList;
    divStyle  : TContentStyle;
  public
    constructor Create(AClassName: String); overload;
    constructor Create; overload;
    destructor Destroy; override;
    
    procedure Append(AClassDIV : TTopicContentDIV); overload;
    procedure Append(AClassName, AContent: String); overload;
    procedure Append(AClassName: String); overload;
  end;

type
  TTopicContent = class(TObject)
  private
    TopicID  : String ;
    TopicCSS : TStringList;
    TopicJS  : TStringList;
    TopicDIV : array of TTopicContentDIV;
  public
    constructor Create;
    destructor Destroy; override;
  end;

type
  TTopic = class(TObject)
  private
    TopicTitle  : String ;
    TopicLevel  : Integer;
    TopicID     : String ;
    TopicContent: TTopicContent;
    TopicEditor : TEditor;
  public
    constructor Create(AName: String); overload;
    constructor Create(AName: String; ALevel: Integer); overload;
    destructor Destroy; override;
    
    procedure LoadFromFile(AFileName: String);
    procedure LoadFromString(AString: String);
    
    procedure MoveRight;
    
    procedure setExternID(AName: String);
    
    function getEditor: TEditor;
    function getID: String;
  end;
procedure TTopic.setExternID(AName: String);
begin
end;

type
  TTemplate = class(TObject)
  end;

type
  TLanguage = class(TObject)
  private
    type FLangEntry = record
        CL: String;
        CN: String;
        CC: String;
        LC: DWORD;
    end;
    const Countries: array[0..39] of FLangEntry = (
        (CL: 'Chinese'; CN: 'Simplified'; CC: 'zh-CHS'; LC: $0004 ),
        (CL: 'Arabic'; CN: 'Saudi Arabia'; CC: 'ar-SA'; LC: $0401 ),
        (CL: 'Bulgarian'; CN: 'Bulgaria'; CC: 'bg-BG'; LC: $0402 ),
        (CL: 'Catalan'; CN: 'Spain'; CC: 'ca-ES'; LC: $0403 ),
        (CL: 'Chinese (Traditional)'; CN: 'Taiwan'; CC: 'zh-TW'; LC: $0404 ),
        (CL: 'Czech'; CN: 'Czech Republic'; CC: 'cs-CZ'; LC: $0405 ),
        (CL: 'Danish'; CN: 'Denmark'; CC: 'da-DK'; LC: $0406 ),
        (CL: 'German'; CN: 'Germany'; CC: 'de-DE'; LC: $0407 ),
        (CL: 'Greek'; CN: ' Greece'; CC: 'el-GR'; LC: $0408 ),
        (CL: 'English'; CN: 'United States'; CC: 'en-US'; LC: $0409 ),
        (CL: 'Spanish'; CN: 'Spain'; CC: 'es-ES_tradnl'; LC: $040A ),
        (CL: 'Finnish'; CN: 'Finland'; CC: 'fi-FI'; LC: $040B ),
        (CL: 'French'; CN: 'France'; CC: 'fr-FR'; LC: $040C ),
        (CL: 'Hebrew'; CN: 'Israel'; CC: 'he-IL'; LC: $040D ),
        (CL: 'Hungarian'; CN: 'Hungary'; CC: 'hu-HU'; LC: $040E ),
        (CL: 'Icelandic'; CN: 'Iceland'; CC: 'is-IS'; LC: $040F ),
        (CL: 'Italian'; CN: 'Italy'; CC: 'it-IT'; LC: $0410 ),
        (CL: 'Japanese'; CN: 'Japan'; CC: 'ja-JP'; LC: $0411 ),
        (CL: 'Korean'; CN: 'Korea'; CC: 'ko-KR'; LC: $0412 ),
        (CL: 'Dutch'; CN: 'Netherlands'; CC: 'nl-NL'; LC: $0413 ),
        (CL: 'Norwegian (Bokmål)'; CN: 'Norway'; CC: 'nb-NO'; LC: $0414 ),
        (CL: 'Polish'; CN: 'Poland'; CC: 'pl-PL'; LC: $0415 ),
        (CL: 'Portuguese'; CN: 'Brazil'; CC: 'pt-BR'; LC: $0416 ),
        (CL: 'Romansh'; CN: 'Switzerland'; CC: 'rm-CH'; LC: $0417 ),
        (CL: 'Romanian'; CN: 'Romania'; CC: 'ro-RO'; LC: $0418 ),
        (CL: 'Russian'; CN: 'Russia'; CC: 'ru-RU'; LC: $0419 ),
        (CL: 'Croatian'; CN: 'Croatia'; CC: 'hr-HR'; LC: $041A ),
        (CL: 'Slovak'; CN: 'Slovakia'; CC: 'sk-SK'; LC: $041B ),
        (CL: 'Albanian'; CN: 'Albania'; CC: 'sq-AL'; LC: $041C ),
        (CL: 'Swedish'; CN: 'Sweden'; CC: 'sv-SE'; LC: $041D ),
        (CL: 'Thai'; CN: 'Thailand'; CC: 'th-TH'; LC: $041E ),
        (CL: 'Turkish'; CN: 'Turkey'; CC: 'tr-TR'; LC: $041F ),
        (CL: 'Urdu'; CN: 'Pakistan'; CC: 'ur-PK'; LC: $0420 ),
        (CL: 'Indonesian'; CN: 'Indonesia'; CC: 'id-ID'; LC: $0421 ),
        (CL: 'Ukrainian'; CN: 'Ukraine'; CC: 'uk-UA'; LC: $0422 ),
        (CL: 'Belarusian'; CN: 'Belarus'; CC: 'be-BY'; LC: $0423 ),
        (CL: 'Slovenian'; CN: 'Slovenia'; CC: 'sl-SI'; LC: $0424 ),
        (CL: 'Estonian'; CN: 'Estonia'; CC: 'et-EE'; LC: $0425 ),
        (CL: 'Latvian'; CN: 'Latvia'; CC: 'lv-LV'; LC: $0426 ),
        (CL: 'Lithuanian'; CN: 'Lithuanian'; CC: 'lt-LT'; LC: $0427 )
    );
  public
    constructor Create(CSL: String); overload;
    constructor Create; overload;
  end;

type
  TProject = class(TObject)
  private
    FLangCode: String;
    Title : String;
    ID : String;
    Topics: Array of TTopic;
    Template: TTemplate;
  public
    constructor Create(AName: String); overload;
    constructor Create; overload;
    destructor Destroy; override;
    
    procedure AddTopic(AName: String; ALevel: Integer); overload;
    procedure AddTopic(AName: String; ATopicID: String); overload;
    procedure AddTopic(AName: String); overload;
    
    procedure SaveToFile(AFileName: String);
    
    procedure SetTemplate(AFileName: String);
    procedure CleanUp;
    
    function getTopic(AListNum: Integer): TTopic;
  published
  property
    LanguageCode: String read FLangCode write FLangCode;
  end;

// ---------------------------------------------------------------------------
// common used constants and variables...
// ---------------------------------------------------------------------------
var HelpNDoc_default: TProject;

// ---------------------------------------------------------------------------
// calculates the indent level of the numbering TOC String
// ---------------------------------------------------------------------------
function GetLevel(const TOCString: String): Integer;
var
  i, count: Integer;
begin
  count := 0;
  // ---------------------------
  // count dot's to get level...
  // ---------------------------
  for i := 1 to Length(TOCString) do
  if TOCString[i] = '.' then
  Inc(count);
  
  // ------------------------------
  // count of dot's is indent level
  // ------------------------------
  Result := count;
end;


{ THtmlContent }

constructor THtmlContent.Create;
begin
  inherited Create;
  Fhtml := TStringList.Create;
  Fhtml.Clear;
end;
destructor THtmlContent.Destroy;
begin
  Fhtml.Clear;
  Fhtml.Free;
  Fhtml := nil;
  
  inherited Destroy;
end;


{ TContentStyle }

constructor TContentStyle.Create(AStyle: String);
begin
  inherited Create;
  styleContent := TStringList.Create;
end;
constructor TContentStyle.Create;
begin
  inherited Create;
  styleContent := TStringList.Create;
end;
destructor TContentStyle.Destroy;
begin
  styleContent.Clear;
  styleContent.Free;
  styleContent := nil;
  
  inherited Destroy;
end;


{ TTopicContentDIV }

constructor TTopicContentDIV.Create(AClassName: String);
begin
  inherited Create;
  divContent := TStringList.Create;
end;
constructor TTopicContentDIV.Create;
begin
  inherited Create;
end;
destructor TTopicContentDIV.Destroy;
begin
  inherited Destroy;
end;

procedure TTopicContentDIV.Append(AClassDIV: TTopicContentDIV);
begin
end;
procedure TTopicContentDIV.Append(AClassName, AContent: String);
begin
end;
procedure TTopicContentDIV.Append(AClassName: String);
begin
end;  


{ TTopicContent }

constructor TTopicContent.Create;
begin
  inherited Create;
  TopicCSS  := TStringList.Create;
  TopicJS   := TStringList.Create;
  
  TopicCSS .Clear;
  TopicJS  .Clear;
end;
destructor TTopicContent.Destroy;
begin
  TopicCSS .Clear; TopicCSS .Free; TopicCSS  := nil;
  TopicJS  .Clear; TopicJS  .Free; TopicJS   := nil;
  
  inherited Destroy;
end;

{ TEditor }

// ---------------------------------------------------------------------------
// \brief This is the constructor for class TEditor. A new Content Editor
//         object will be created. The default state is empty.
// ---------------------------------------------------------------------------
constructor TEditor.Create;
begin
  inherited Create;

  if FileExists(ROOT_HND) then  // todo !!!
  begin
    if ROOT_HND_DEL = 1 then
    begin
      try
        print("delete existing project !");
        DeleteFile(ROOT_HND);
      except
        on E: Exception do
        raise Exception.Create('can not delete project file.');
      end;
    end;
  end;

  ID := HndEditor.CreateTemporaryEditor;
  Clear;
end;

// ---------------------------------------------------------------------------
// \brief This is the destructor for class EDitor. Here, we try to remove so
//         much memory as possible that was allocated before.
// ---------------------------------------------------------------------------
destructor TEditor.Destroy;
begin
  Clear;
  HndEditor.DestroyTemporaryEditor(ID);
  inherited Destroy;
end;

// ---------------------------------------------------------------------------
// \brief This function make the current content editor clean for new input.
// ---------------------------------------------------------------------------
procedure TEditor.Clear;
begin
  if not Assigned(ID) then
  raise Exception.Create('Editor not created.');
  
  HndEditorHelper.CleanContent(getID);
  HndEditor.Clear(getID);
end;

// ---------------------------------------------------------------------------
// \brief This function loads the HTML Content for the current content editor
//         Warning: Existing Code will be overwrite through this function !
// ---------------------------------------------------------------------------
procedure TEditor.LoadFromFile(AFileName: String);
var strList: TStringList;
begin
  if not Assigned(ID) then
  raise Exception.Create('Error: Editor ID unknown.');
  try
    try
      strList := TStringList.Create;
      strList.LoadFromFile(AFileName);
      Content := Trim(strList.Text);
      
      HndEditor.InsertContentFromHTML(getID, Content);
    except
      on E: Exception do
      raise Exception.Create('Error: editor content can not load from file.');
    end;
  finally
    strList.Clear;
    strList.Free;
    strList := nil;
  end;
end;

// ---------------------------------------------------------------------------
// \brief This function load the HTML Content for the current Content Editor
//         by the given AString HTML code.
//         Warning: Existing Code will be overwrite throug this function !
// ---------------------------------------------------------------------------
procedure TEditor.LoadFromString(AString: String);
begin
  if not Assigned(getID) then
  raise Exception.Create('Error: editor ID unknown.');
  try
    Content := Trim(AString);
    HndEditor.InsertContentFromHTML(getID, AString);
  except
    on E: Exception do
    raise Exception.Create('Error: editor content could not set.');
  end;
end;

procedure TEditor.SaveToFile(AFileName: String);
begin
  //GetContentAsHtml()
end;

function  TEditor.getContent: String ; begin result := Content; end;
function  TEditor.getID     : TObject; begin result := ID;      end;

procedure TEditor.setContent(AString: String);
begin
  Content := AString;
  HndEditor.InsertContentFromHTML(getID, getContent);
end;

{ TTopic }

// ---------------------------------------------------------------------------
// \brief This is the constructor for class TTopic. It creates a new fresh
//         Topic with given AName and a indent with ALevel.
// ---------------------------------------------------------------------------
constructor TTopic.Create(AName: String; ALevel: Integer);
begin
  inherited Create;
  
  TopicTitle  := AName;
  TopicLevel  := ALevel;
  TopicID     := HndTopics.CreateTopic;
  
  HndTopics.SetTopicCaption(TopicID, TopicTitle);
  MoveRight;
  
  TopicEditor := TEditor.Create;
end;

// ---------------------------------------------------------------------------
// \brief This is a overloaded constructor for class TTopic. It creates a new
//         fresh Topic if the given AName, and a indent which is automatically
//         filled in.
// ---------------------------------------------------------------------------
constructor TTopic.Create(AName: String);
begin
  inherited Create;
  
  TopicTitle  := AName;
  TopicLevel  := GetLevel(TopicTitle);
  TopicID     := HndTopics.CreateTopic;
  
  HndTopics.SetTopicCaption(TopicID, TopicTitle);
  MoveRight;
  
  TopicEditor  := TEditor.Create;
  TopicContent := TTopicContent.Create;
end;

// ---------------------------------------------------------------------------
// \brief This is the destructor for class TTopic. Here we try to remove so
//         much memory as possible is allocated before.
// ---------------------------------------------------------------------------
destructor TTopic.Destroy;
begin
  TopicContent.Free; TopicContent := nil;
  TopicEditor .Free; TopicEditor  := nil;
  
  inherited Destroy;
end;

// ---------------------------------------------------------------------------
// \brief This is a place holder function to reduce code redundance.
// ---------------------------------------------------------------------------
procedure TTopic.MoveRight;
var idx: Integer;
begin
  if TopicLevel > 1 then
  begin
    for idx := 1 to TopicLevel do
    HndTopics.MoveTopicRight(TopicID);
  end;
end;

// ---------------------------------------------------------------------------
// \brief This function loads the Topic Content from a File and fill it into
//         the Content Editor.
// ---------------------------------------------------------------------------
procedure TTopic.LoadFromFile(AFileName: String);
var strList: TStringList;
begin
  try
    try
      strList := TStringList.Create;
      strList.LoadFromFile(AFileName);
      getEditor.setContent(Trim(strList.Text));
    except
      on E: Exception do
      raise Exception.Create('Error: editor content can not load from file.');
    end;
  finally
    strList.Clear;
    strList.Free;
    strList := nil;
  end;
end;

procedure TTopic.LoadFromString(AString: String);
begin
end;

function TTopic.getEditor: TEditor; begin result := TopicEditor; end;
function TTopic.getID    : String ; begin result := TopicID;     end;

{ TLanguage }

constructor TLanguage.Create;
begin
    inherited Create;
end;

constructor TLanguage.Create(CSL: String);
begin
    inherited Create;
end;


{ TProject }

// ---------------------------------------------------------------------------
// \brief This is the constructor for class TProject. It creates a new fresh
//         Project with the given AName.
// ---------------------------------------------------------------------------
constructor TProject.Create(AName: String);
begin
  inherited Create;
  
  try
    HndProjects.CloseProject;
    DeleteFile(AName);
    
    Title     := AName;
    ID        := HndProjects.NewProject(AName);
    FLangCode := 'en-us';
    
    HndProjects.SetProjectModified(True);
    HndProjects.SetProjectLanguage(850);
    HndProjects.SaveProject;
  except
    //on E: EIoOutException do
    //raise Exception.Create('Project file already in use.');
    on E: Exception do
    raise Exception.Create('Project file could not be created.');
  end;
end;

// ---------------------------------------------------------------------------
// \brief This is the overloaded constructor to create a new Project with the
//         default settings.
// ---------------------------------------------------------------------------
constructor TProject.Create;
begin
  inherited Create;
  
  try
    Title     := 'default.hnd';
    ID        := HndProjects.NewProject(Title);
    FLangCode := 'en-us';
    
    HndProjects.SetProjectModified(True);
    HndProjects.SetProjectLanguage(850);
    HndProjects.SaveProject;
  except
    on E: Exception do
    raise Exception.Create('Error: project could not be loaded.' + #10 + E.Message);
  end;
end;

// ---------------------------------------------------------------------------
// \brief This is the destructor of class TProject. Here we try to remove so
//         much memory as possible is allocated before.
// ---------------------------------------------------------------------------
destructor TProject.Destroy;
var index: Integer;
begin

    HndProjects.SaveProject;
    HndProjects.CloseProject;
    
    CleanUp;
    inherited Destroy;
end;

procedure TProject.CleanUp;
var index: Integer;
begin
  for index := High(Topics) downto Low(Topics) do
  begin
    Topics[index].Free;
    Topics[index] := nil;
  end;
  Topics := nil;
end;

function TProject.getTopic(AListNum: Integer): TTopic;
begin
  if Topics.Count < 1 then
  raise Exception.Create('Project has no topics.');
  
  if Topics.Count-1 < AListNum then
  raise Exception.Create('Project has no topics.');
  
  result := Topics[AListNum];
end;

// ---------------------------------------------------------------------------
// \brief This function save the HTML Content and Project Data to storage.
// ---------------------------------------------------------------------------
procedure TProject.SaveToFile(AFileName: String);
begin
  if Length(Trim(ID)) < 1 then
  raise Exception.Create('Error: Project ID is nil.');
  
  if Length(Trim(AFileName)) > 0 then
  begin
    HndProjects.SaveProject;
    HndProjects.CloseProject;
    HndProjects.CopyProject(AFileName, false);
  end;
end;

// ---------------------------------------------------------------------------
// \brief add an new Topic with AName and ALevel
// ---------------------------------------------------------------------------
procedure TProject.AddTopic(AName: String; ALevel: Integer);
var
  Topic: TTopic;
begin
  try
    Topic  := TTopic.Create(AName, ALevel);
    HndEditor.SetAsTopicContent(Topic.getEditor.getID, Topic.getID);
    Topics := Topics + [Topic];
  except
    on E: Exception do
    raise Exception.Create('Error: can not create topic.');
  end;
end;

procedure TProject.AddTopic(AName: String; ATopicID: String);
var Topic: TTopic;
begin
  try
    Topic  := TTopic.Create(AName, GetLevel(AName));
    Topic.setExternID(ATopicID);
    HndEditor.SetAsTopicContent(Topic.getEditor.getID, Topic.getID);
    Topics := Topics + [Topic];
  except
    on E: Exception do
    raise Exception.Create('Error: can not create topic.');
  end;
end;

// ---------------------------------------------------------------------------
// \brief add a new Topic with AName. the level is getting by GetLevel
// ---------------------------------------------------------------------------
procedure TProject.AddTopic(AName: String);
var
  Topic: TTopic;
begin
  try
    Topic  := TTopic.Create(AName, GetLevel(AName));
    HndEditor.SetAsTopicContent(Topic.getEditor.getID, Topic.getID);
    Topics := Topics + [Topic];
  except
    on E: Exception do
    raise Exception.Create('Error: can not create topic.');
  end;
end;

procedure TProject.SetTemplate(AFileName: String);
begin
end;

// ---------------------------------------------------------------------------
// \brief This function extracts the Topic Caption/Titel of the given String.
// ---------------------------------------------------------------------------
function ExtractTitel(const TOCString: String): String;
var
  posSpace: Integer;
begin
  // -------------------------------------
  // find white space after numbering ...
  // -------------------------------------
  posSpace := Pos(' ', TOCString);
  if posSpace > 0 then
  Result := Copy(TOCString, posSpace + 1, Length(TOCString)) else
  
  // --------------------
  // if no white space...
  // --------------------
  Result := TOCString;
end;

// ---------------------------------------------------------------------------
// \brief  This function create a fresh new Project. If a Project with the
//         name already exists, then it will be overwrite !
//
// \param  projectName - String: The name of the Project.
// ---------------------------------------------------------------------------
procedure CreateProject(const projectName: String);
var projectID: String;
begin
  HelpNDoc_default := TProject.Create(projectName);
end;

procedure DocItemInit(var d: TDocItem);
begin
  d.Notes       := TStringList.Create; d.Notes     .Clear;
  d.Warnings    := TStringList.Create; d.Warnings  .Clear;
  d.Sees        := TStringList.Create; d.Sees      .Clear;
  d.ParamNames  := TStringList.Create; d.ParamNames.Clear;
  d.ParamTexts  := TStringList.Create; d.ParamTexts.Clear;
end;

procedure DocItemFree(var d: TDocItem);
begin
  if d.Notes      <> nil then begin d.Notes     .Clear; d.Notes     .Free; end;
  if d.Warnings   <> nil then begin d.Warnings  .Clear; d.Warnings  .Free; end;
  if d.Sees       <> nil then begin d.Sees      .Clear; d.Sees      .Free; end;
  if d.ParamNames <> nil then begin d.ParamNames.Clear; d.ParamNames.Free; end;
  if d.ParamTexts <> nil then begin d.ParamTexts.Clear; d.ParamTexts.Free; end;

  d.Notes      := nil;
  d.Warnings   := nil;
  d.Sees       := nil;
  d.ParamNames := nil;
  d.ParamTexts := nil;
end;

function DocItemClone(const src: TDocItem): TDocItem;
begin
  // einfache Felder kopieren
  Result := src;

  // aber: Listen deep-clonen (neue Objekte!)
  Result.Notes      := TStringList.Create; Result.Notes     .Text := src.Notes     .Text;
  Result.Warnings   := TStringList.Create; Result.Warnings  .Text := src.Warnings  .Text;
  Result.Sees       := TStringList.Create; Result.Sees      .Text := src.Sees      .Text;
  Result.ParamNames := TStringList.Create; Result.ParamNames.Text := src.ParamNames.Text;
  Result.ParamTexts := TStringList.Create; Result.ParamTexts.Text := src.ParamTexts.Text;
end;

// -------------------------
// small string helpers
// -------------------------
function TrimLeft(const s: string): string;
begin
  Result := s;
  while (Length(Result) > 0) and (Result[1] <= ' ') do Delete(Result, 1, 1);
end;

function TrimRight(const s: string): string;
begin
  Result := s;
  while (Length(Result) > 0) and (Result[Length(Result)] <= ' ') do Delete(Result, Length(Result), 1);
end;

function StartsWith(const s, prefix: string): Boolean;
begin
  Result := (Copy(s, 1, Length(prefix)) = prefix);
end;

function IsIdentifierChar(ch: Char): Boolean;
begin
  Result := (ch in ['A'..'Z','a'..'z','0'..'9','_']);
end;

procedure AddStr(var arr: TStringArray; const s: string);
begin
  if arr = nil then
     arr := TStringArray.Create;
     arr.add(s);
end;

procedure AddParam(var item: TDocItem; const pname, ptext: string);
begin
  if item.ParamNames = nil then item.ParamNames := TStringArray.Create;
  if item.ParamTexts = nil then item.ParamTexts := TStringArray.Create;
   	 
  item.ParamNames.Add(String(pname));
  item.ParamTexts.Add(String(ptext));
end;

function ExtractLastIdentifierBeforeParen(const line: string): string;
var
  i, p: Integer;
begin
  Result := '';
  p := Pos('(', line);
  if p <= 1 then Exit;

  i := p - 1;
  while (i > 0) and (line[i] <= ' ') do Dec(i);
  while (i > 0) and (line[i] in ['*','&']) do Dec(i);
  while (i > 0) and (line[i] <= ' ') do Dec(i);

  while (i > 0) and IsIdentifierChar(line[i]) do
  begin
    Result := line[i] + Result;
    Dec(i);
  end;
end;

function HtmlEscape(const s: string): string;
var i: Integer; ch: Char;
begin
  Result := '';
  for i := 1 to Length(s) do
  begin
    ch := s[i];
    case ch of
      '&': Result := Result + '&amp;';
      '<': Result := Result + '&lt;';
      '>': Result := Result + '&gt;';
      '"': Result := Result + '&quot;';
    else
      Result := Result + ch;
    end;
  end;
end;

// -------------------------
// file loading
// -------------------------
function LoadAllLines(const filename: string): TStringArray;
var
  sl: TStringList;
  i: Integer;
begin
  sl := TStringList.Create;
  try
    sl.LoadFromFile(filename);
    Result := sl;
    for i := 0 to sl.Count - 1 do
      Result.add(sl[i]);
  finally
  end;
end;

// -------------------------
// doc parsing helpers
// -------------------------
function NormalizeDocLine_Block(const s: string): string;
var t: string;
begin
  t := TrimLeft(s);

  if StartsWith(t, '/**') then
  begin
    Delete(t, 1, 3);
    t := TrimLeft(t);
  end;

  if StartsWith(t, '*') then
  begin
    Delete(t, 1, 1);
    if (Length(t) > 0) and (t[1] = ' ') then Delete(t, 1, 1);
  end;

  if Pos('*/', t) > 0 then
    t := TrimRight(Copy(t, 1, Pos('*/', t) - 1));

  Result := TrimRight(t);
end;

function NormalizeDocLine_Slash(const s: string): string;
var t: string;
begin
  t := TrimLeft(s);
  if StartsWith(t, '///') then
  begin
    Delete(t, 1, 3);
    if (Length(t) > 0) and (t[1] = ' ') then Delete(t, 1, 1);
  end;
  Result := TrimRight(t);
end;

procedure ParseDocLines(const rawLines: TStringArray; var item: TDocItem; slashStyle: Boolean);
var
  i, sp: Integer;
  line, rest, pname, ptext: string;
begin
  item.GroupName := 'General';
  item.Brief := '';
  item.ReturnText := '';

  for i := 0 to rawLines.count - 1 do
  begin
    if slashStyle then line := NormalizeDocLine_Slash(rawLines[i])
                  else line := NormalizeDocLine_Block(rawLines[i]);

    line := Trim(line);
    if line = '' then Continue;

    if StartsWith(line, '@ingroup') then
    begin
      rest := Trim(Copy(line, Length('@ingroup')+1, 9999));
      if rest <> '' then item.GroupName := rest;
      Continue;
    end;

    if StartsWith(line, '@brief') then
    begin
      rest := Trim(Copy(line, Length('@brief')+1, 9999));
      if item.Brief = '' then item.Brief := rest else item.Brief := item.Brief + ' ' + rest;
      Continue;
    end;

    if StartsWith(line, '@param') then
    begin
      rest := Trim(Copy(line, Length('@param')+1, 9999));
      sp := Pos(' ', rest);
      if sp > 0 then
      begin
        pname := Copy(rest, 1, sp-1);
        ptext := Trim(Copy(rest, sp+1, 9999));
      end else begin
        pname := rest;
        ptext := '';
      end;
      AddParam(item, pname, ptext);
      Continue;
    end;

    if StartsWith(line, '@return') then
    begin
      rest := Trim(Copy(line, Length('@return')+1, 9999));
      if item.ReturnText = '' then item.ReturnText := rest else item.ReturnText := item.ReturnText + ' ' + rest;
      Continue;
    end;

    if StartsWith(line, '@note') then
    begin
      rest := Trim(Copy(line, Length('@note')+1, 9999));
      AddStr(item.Notes, rest);
      Continue;
    end;

    if StartsWith(line, '@warning') then
    begin
      rest := Trim(Copy(line, Length('@warning')+1, 9999));
      AddStr(item.Warnings, rest);
      Continue;
    end;

    if StartsWith(line, '@see') then
    begin
      rest := Trim(Copy(line, Length('@see')+1, 9999));
      AddStr(item.Sees, rest);
      Continue;
    end;

    // Default -> brief
    if item.Brief = '' then item.Brief := line else item.Brief := item.Brief + ' ' + line;
  end;
end;

function IsSkippableLine(const s: string): Boolean;
var t: string;
begin
  t := TrimLeft(s);
  Result :=
    (t = '') or
    StartsWith(t, '#include') or StartsWith(t, '#define') or
    StartsWith(t, '//') or StartsWith(t, '/*') or
    StartsWith(t, 'extern') or StartsWith(t, 'static') or StartsWith(t, 'inline') or
    StartsWith(t, '__attribute__') or StartsWith(t, '[[');
end;

function DetectKindAndName(const sig: string; var kind, name: string): Boolean;
var t, tmp: string; p: Integer;
begin
  Result := False;
  kind := '';
  name := '';
  t := TrimLeft(sig);

  if StartsWith(t, 'class ')
  or StartsWith(t, 'struct ')
  or StartsWith(t, 'enum ')
  or StartsWith(t, 'namespace ')
  then
  begin
    kind := 'type';
    if StartsWith(t, 'class '    ) then tmp := Trim(Copy(t,  7, 9999));
    if StartsWith(t, 'struct '   ) then tmp := Trim(Copy(t,  8, 9999));
    if StartsWith(t, 'enum '     ) then tmp := Trim(Copy(t,  6, 9999));
	if StartsWith(t, 'namespace ') then tmp := Trim(Copy(t, 11, 9999));

    name := '';
    p := 1;
    while (p <= Length(tmp)) and IsIdentifierChar(tmp[p]) do
    begin
      name := name + tmp[p];
      Inc(p);
    end;
    Result := (name <> '');
    Exit;
  end;

  if (Pos('(', t) > 0) and (Pos(')', t) > 0) then
  begin
    if StartsWith(t, 'if')
	or StartsWith(t, 'for')
	or StartsWith(t, 'while')
	or StartsWith(t, 'switch')
	then Exit;
    kind := 'function';
    name := ExtractLastIdentifierBeforeParen(t);
    Result := (name <> '');
  end;
end;

function BuildTopicHtml(item: TDocItem): string;
var i: Integer;
begin
  result := '';
  if item.runningPDF = true then
  result := '<style>.back { background: white; color: black; }</style>' else
  result := '<style>.back { background: black; color: yellow;}</style>';
  
  result := result + '<div class="back"><h1>' + HtmlEscape(item.Name) + '</h1>';

  if item.Signature <> '' then
    result := result + '<pre style="font-family: Consolas, monospace; font-size: 10pt;">' +
              HtmlEscape(item.Signature) + '</pre>';

  if item.Brief <> '' then
    Result := Result + '<p>' + HtmlEscape(item.Brief) + '</p>';

//todo
  if item.ParamNames.count > 0 then
  begin
    Result := Result + '<h2>Parameters</h2><ul>';
    for i := 0 to item.ParamNames.count-1 do
      Result := Result + '<li><b>' + HtmlEscape(item.ParamNames[i]) + ':</b> ' +
                HtmlEscape(item.ParamTexts[i]) + '</li>';
    Result := Result + '</ul>';
  end;

  if item.ReturnText <> '' then
    Result := Result + '<h2>Returns</h2><p>' + HtmlEscape(item.ReturnText) + '</p>';
//todo
  if item.Notes.count > 0 then
  begin
    Result := Result + '<h2>Notes</h2><ul>';
    for i := 0 to item.Notes.count-1 do
      Result := Result + '<li>' + HtmlEscape(item.Notes[i]) + '</li>';
    Result := Result + '</ul>';
  end;
//todo
  if item.Warnings = nil then item.Warnings := TStringList.Create; 
  if item.Warnings.count > 0 then
  begin
    Result := Result + '<h2>Warnings</h2><ul>';
    for i := 0 to item.Warnings.count-1 do
      Result := Result + '<li>' + HtmlEscape(item.Warnings[i]) + '</li>';
    Result := Result + '</ul>';
  end;

  if item.Sees.count > 0 then
  begin
    Result := Result + '<h2>See also</h2><ul>';
    for i := 0 to item.Sees.count-1 do
      Result := Result + '<li>' + HtmlEscape(item.Sees[i]) + '</li>';
    Result := Result + '</ul>';
  end;
  
  result := result + '</div>';
end;

// -------------------------
// HelpNDoc: create topic + fill HTML
// -------------------------
procedure CreateTopicWithHtml(const caption, html: string; item: TDocItem);
var
  topicId, htmlstr: string;
  ed: TObject;
begin
  if currenttopic = caption then exit else
  currenttopic := caption;

  topicId := HndTopics.CreateTopic;
  HndTopics.SetCurrentTopic(topicId);
  HndTopics.SetTopicCaption(topicId, caption);

  ed := HndEditor.CreateTemporaryEditor;
  try
    // Topic-Inhalt in den Editor laden
	item.runningPDF := false;
	htmlstr := BuildTopicHtml(item);
    HndEditor.InsertTopicContent(ed, topicId);
	HndEditor.InsertCondition(ed, 'IF', 'CHM');
	HndEditor.InsertContentFromHTML(ed, html);
	HndEditor.InsertCondition(ed, 'ENDIF', '');
	
	item.runningPDF := true;
	htmlstr := BuildTopicHtml(item);
	HndEditor.InsertCondition(ed, 'IF', 'PDF');
	HndEditor.InsertContentFromHTML(ed, html);
	HndEditor.InsertCondition(ed, 'ENDIF', '');
	HndEditor.SetAsTopicContent(ed, topicId);
  finally
    HndEditor.DestroyTemporaryEditor(ed);
  end;
end;

procedure InitDocItemLists(
	var notes,
	warnings,
	sees,
	pnames,
	ptexts: TStringList);
begin
  notes.Clear;
  warnings.Clear;
  sees.Clear;
  pnames.Clear;
  ptexts.Clear;
end;
// -------------------------
// Process one file: supports /** */ and ///
// -------------------------
procedure ProcessFile(const filename: string);
var
  lines, docLines: TStringArray;
  i, j: Integer;
  inBlock: Boolean;
  inSlash: Boolean;
  sig, kind, name: string;
  item: TDocItem;
  html, caption: string;
begin
  lines := LoadAllLines(filename);

  docLines := TStringArray.Create;
  inBlock := False;
  inSlash := False;
  docLines.clear;

  i := 0;
  while i <= lines.count-1 do
  begin
    // start /** */
    if (not inBlock) and (not inSlash) and (Pos('/**', lines[i]) > 0) then
    begin
      inBlock := True;
      docLines.clear;
      AddStr(docLines, lines[i]);
      Inc(i);
      Continue;
    end;

    // inside /** */
    if inBlock then
    begin
      AddStr(docLines, lines[i]);
      if Pos('*/', lines[i]) > 0 then
      begin
        inBlock := False;

        // next symbol line
        j := i + 1;
        while (j <= lines.count-1) and IsSkippableLine(lines[j]) do Inc(j);

        if j <= lines.count-1 then
        begin
          sig := Trim(lines[j]);
          while (j < lines.count-1) and (Pos(';', sig) = 0) and (Pos('{', sig) = 0) and (Length(sig) < 500) do
          begin
            Inc(j);
            if IsSkippableLine(lines[j]) then Break;
            sig := sig + ' ' + Trim(lines[j]);
          end;

          if DetectKindAndName(sig, kind, name) then
          begin
            //FillChar(item, SizeOf(item), 0);
			//InitDocItem(item);
            item.Kind := kind;
            item.Name := name;
            item.Signature := sig;
            item.SourceFile := filename;
            item.LineNo := j;
			item.GroupName := 'General';
			item.Brief := '';

            ParseDocLines(docLines, item, False);
            html := BuildTopicHtml(item);

            // Caption (einmalig): optional Group prefix
            if item.GroupName <> 'General' then caption := item.GroupName + ' :: ' + item.Name
                                         else caption := item.Name;

            CreateTopicWithHtml(caption, html, item);
          end;
        end;
      end;

      Inc(i);
      Continue;
    end;

    // start /// block (one or more consecutive /// lines)
    if (not inSlash) and (StartsWith(TrimLeft(lines[i]), '///')) then
    begin
      inSlash := True;
      docLines.clear;
      AddStr(docLines, lines[i]);
      Inc(i);
      // collect consecutive ///
      while (i <= lines.count-1) and StartsWith(TrimLeft(lines[i]), '///') do
      begin
        AddStr(docLines, lines[i]);
        Inc(i);
      end;

      // now i is first non-/// line -> find next symbol line
      j := i;
      while (j <= lines.count-1) and IsSkippableLine(lines[j]) do Inc(j);

      if j <= lines.count-1 then
      begin
        sig := Trim(lines[j]);
        while (j < lines.count-1) and (Pos(';', sig) = 0) and (Pos('{', sig) = 0) and (Length(sig) < 500) do
        begin
          Inc(j);
          if IsSkippableLine(lines[j]) then Break;
          sig := sig + ' ' + Trim(lines[j]);
        end;

        if DetectKindAndName(sig, kind, name) then
        begin
		  //FillChar(item, SizeOf(item), 0);
          item.Kind := kind;
          item.Name := name;
          item.Signature := sig;
          item.SourceFile := filename;
          item.LineNo := j;

          ParseDocLines(docLines, item, True);
          html := BuildTopicHtml(item);

          if item.GroupName <> 'General'
		  then caption := item.GroupName + ' :: ' + item.Name else
		  caption := item.Name;
		  
          CreateTopicWithHtml(caption, html, item);
        end;
      end;

      inSlash := False;
      Continue;
    end;

    Inc(i);
  end;
end;

// -------------------------
// File iteration (Punkt 2)
// -------------------------
function HasExt(const fn, ext: string): Boolean;
begin
  Result := (LowerCase(Copy(fn, Length(fn)-Length(ext)+1, Length(ext))) = LowerCase(ext));
end;

function ShouldProcessFile(const fn: string): Boolean;
begin
  if ONLY_HEADERS then
    Result := HasExt(fn, '.h') or HasExt(fn, '.hpp')
  else if INCLUDE_CPP then
    Result := HasExt(fn, '.h') or HasExt(fn, '.hpp') or HasExt(fn, '.c') or HasExt(fn, '.cpp')
  else
    Result := HasExt(fn, '.h') or HasExt(fn, '.hpp') or HasExt(fn, '.c');
end;

// -------------------------
// Main
// -------------------------
procedure RunHnd;
const
  fn = ROOT_DIR + '\TPoint.h'; 
begin
  // Start:
  print('start...');
  if ShouldProcessFile(fn) = true then
  begin
    ProcessFile(fn);
  end; 
end;

// ---------------------------------------------------------------------------
// \brief This function create the Table of Contents (TOC).
// ---------------------------------------------------------------------------
procedure CreateTableOfContents;
var i, p, g: Integer;
begin
  HelpNDoc_default := TProject.Create(ROOT_HND);
  try
    print('1. pre-processing data...');
    HelpNDoc_default.SetTemplate(HelpNDocTemplateHTM);

    HelpNDoc_default.AddTopic('Lizenz - Bitte lesen !!!');
    HelpNDoc_default.AddTopic('Überblich');
    HelpNDoc_default.AddTopic('Inhalt');
    HelpNDoc_default.AddTopic('Liste der Tabellen');
    HelpNDoc_default.AddTopic('Über dieses Handbuch');
    HelpNDoc_default.AddTopic('Bezeichnungen');
    HelpNDoc_default.AddTopic('Syntax Diagramme');
    HelpNDoc_default.AddTopic('Über die Sprache Pascal');
    HelpNDoc_default.AddTopic('1.  Pascal Zeichen und Symbole');
    HelpNDoc_default.AddTopic('1.1  Symbole');
    HelpNDoc_default.AddTopic('1.2  Kommentare');
    HelpNDoc_default.AddTopic('1.3  Reservierte Schlüsselwörter');
    HelpNDoc_default.AddTopic('1.3.1.  Turbo Pascal');
    HelpNDoc_default.AddTopic('1.3.2.  Object Pascal');
    HelpNDoc_default.AddTopic('1.3.3.  Modifikationen');
    HelpNDoc_default.AddTopic('1.4.  Kennzeichnungen');
    HelpNDoc_default.AddTopic('1.5.  Hinweise und Direktiven');
    HelpNDoc_default.AddTopic('1.6.  Zahlen');
    HelpNDoc_default.AddTopic('1.7.  Bezeichner');
    HelpNDoc_default.AddTopic('1.8.  Zeichenketten');
    HelpNDoc_default.AddTopic('2.  Konstanten');
    HelpNDoc_default.AddTopic('2.1.  Gewöhnliche Konstanten');
    HelpNDoc_default.AddTopic('2.2.  Typisierte Konstanten');
    HelpNDoc_default.AddTopic('2.3.  Resourcen Zeichenketten');
    HelpNDoc_default.AddTopic('3.  Typen');

  finally
    print('3.  clean up memory...');
    
    HelpNDoc_default.Free;
    HelpNDoc_default := nil;
    
    print('4.  done.');
  end;
end;

procedure Run;
begin
  try
    try
      CreateTableOfContents;
      ShowMessage('Project successfully created.');
    except
      on E: Exception do
      begin
        ShowMessage('Error:' + #13#10 + E.Message);
      end;
    end;
  finally
  end;
end;

begin
  Run;
end.
